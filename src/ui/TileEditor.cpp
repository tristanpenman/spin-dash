#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QListWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>

#include "../Block.h"
#include "../Chunk.h"
#include "../Level.h"
#include "../Palette.h"
#include "../Pattern.h"

#include "TileEditor.h"

using namespace std;

static constexpr int CANVAS_SCALE = 3;
static constexpr int BLOCK_PREVIEW_SCALE = 2;
static constexpr uint16_t H_FLIP_MASK = 0x400;
static constexpr uint16_t V_FLIP_MASK = 0x800;

static QColor toQColor(const Palette::Color& color)
{
  return QColor(color.r, color.g, color.b);
}

TileCanvas::TileCanvas(QWidget* parent, shared_ptr<Level>& level)
  : QWidget(parent)
  , m_level(level)
  , m_tileIndex(0)
  , m_previewPaletteIndex(0)
  , m_selectedBlockIndex(0)
  , m_hFlip(false)
  , m_vFlip(false)
  , m_highlightX(-1)
  , m_highlightY(-1)
{
  setFixedSize(Block::BLOCK_WIDTH * CANVAS_SCALE, Block::BLOCK_HEIGHT * CANVAS_SCALE);
  setMouseTracking(true);
}

void TileCanvas::setTileIndex(size_t tileIndex)
{
  m_tileIndex = tileIndex;
  update();
}

void TileCanvas::setPreviewPalette(size_t paletteIndex)
{
  m_previewPaletteIndex = paletteIndex;
  update();
}

void TileCanvas::setSelectedBlock(uint16_t blockIndex)
{
  m_selectedBlockIndex = blockIndex;
}

void TileCanvas::setHorizontalFlip(bool enabled)
{
  m_hFlip = enabled;
}

void TileCanvas::setVerticalFlip(bool enabled)
{
  m_vFlip = enabled;
}

void TileCanvas::mouseMoveEvent(QMouseEvent* event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  const QPoint pos = event->position().toPoint();
#else
  const QPoint pos = event->pos();
#endif
  m_highlightX = pos.x() / (Chunk::CHUNK_WIDTH * CANVAS_SCALE);
  m_highlightY = pos.y() / (Chunk::CHUNK_HEIGHT * CANVAS_SCALE);
  if (m_highlightX < 0 || m_highlightX >= 8 || m_highlightY < 0 || m_highlightY >= 8) {
    m_highlightX = -1;
    m_highlightY = -1;
  }

  if (event->buttons() & Qt::LeftButton) {
    drawAt(pos);
  } else {
    update();
  }
}

void TileCanvas::mousePressEvent(QMouseEvent* event)
{
  if (event->button() != Qt::LeftButton) {
    return;
  }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  drawAt(event->position().toPoint());
#else
  drawAt(event->pos());
#endif
}

void TileCanvas::paintEvent(QPaintEvent*)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, false);
  painter.scale(CANVAS_SCALE, CANVAS_SCALE);
  painter.fillRect(QRect(0, 0, Block::BLOCK_WIDTH, Block::BLOCK_HEIGHT), Qt::black);

  drawBlock(painter, m_level->getBlock(m_tileIndex));

  painter.setPen(QColor(55, 55, 55));
  for (int i = 0; i <= 8; i++) {
    painter.drawLine(i * Chunk::CHUNK_WIDTH, 0, i * Chunk::CHUNK_WIDTH, Block::BLOCK_HEIGHT);
    painter.drawLine(0, i * Chunk::CHUNK_HEIGHT, Block::BLOCK_WIDTH, i * Chunk::CHUNK_HEIGHT);
  }

  if (m_highlightX >= 0 && m_highlightY >= 0) {
    painter.fillRect(m_highlightX * Chunk::CHUNK_WIDTH,
                     m_highlightY * Chunk::CHUNK_HEIGHT,
                     Chunk::CHUNK_WIDTH,
                     Chunk::CHUNK_HEIGHT,
                     QColor(128, 192, 255, 64));
  }
}

void TileCanvas::drawAt(const QPoint& pos)
{
  const int x = pos.x() / (Chunk::CHUNK_WIDTH * CANVAS_SCALE);
  const int y = pos.y() / (Chunk::CHUNK_HEIGHT * CANVAS_SCALE);
  if (x < 0 || x >= 8 || y < 0 || y >= 8) {
    return;
  }

  uint16_t value = m_selectedBlockIndex & 0x3FF;
  if (m_hFlip) {
    value |= H_FLIP_MASK;
  }
  if (m_vFlip) {
    value |= V_FLIP_MASK;
  }

  Block& tile = m_level->getBlock(m_tileIndex);
  if (tile.getChunkDesc(static_cast<uint8_t>(x), static_cast<uint8_t>(y)).get() == value) {
    return;
  }

  tile.setChunkDesc(static_cast<uint8_t>(x), static_cast<uint8_t>(y), value);
  update();
  emit tileModified();
}

void TileCanvas::drawBlock(QPainter& painter, const Block& block)
{
  for (int dy = 0; dy < 8; dy++) {
    for (int dx = 0; dx < 8; dx++) {
      const auto& chunkDesc = block.getChunkDesc(static_cast<uint8_t>(dx), static_cast<uint8_t>(dy));
      try {
        const auto& chunk = m_level->getChunk(chunkDesc.getChunkIndex());
        drawChunk(painter, chunk, dx * Chunk::CHUNK_WIDTH, dy * Chunk::CHUNK_HEIGHT, chunkDesc.getHFlip(), chunkDesc.getVFlip());
      } catch (...) {
      }
    }
  }
}

void TileCanvas::drawChunk(QPainter& painter, const Chunk& chunk, int dx, int dy, bool hFlip, bool vFlip)
{
  for (int py = 0; py < 2; py++) {
    for (int px = 0; px < 2; px++) {
      const auto& patternDesc = chunk.getPatternDesc(hFlip ? 1 - px : px, vFlip ? 1 - py : py);
      const auto& pattern = m_level->getPattern(patternDesc.getPatternIndex());
      const auto& palette = m_level->getPalette(m_previewPaletteIndex);
      drawPattern(painter,
                  pattern,
                  palette,
                  dx + px * Pattern::PATTERN_WIDTH,
                  dy + py * Pattern::PATTERN_HEIGHT,
                  patternDesc.getHFlip() ^ hFlip,
                  patternDesc.getVFlip() ^ vFlip);
    }
  }
}

void TileCanvas::drawPattern(QPainter& painter,
                             const Pattern& pattern,
                             const Palette& palette,
                             int dx,
                             int dy,
                             bool hFlip,
                             bool vFlip)
{
  for (int py = 0; py < Pattern::PATTERN_HEIGHT; py++) {
    for (int px = 0; px < Pattern::PATTERN_WIDTH; px++) {
      const auto fx = hFlip ? Pattern::PATTERN_WIDTH - 1 - px : px;
      const auto fy = vFlip ? Pattern::PATTERN_HEIGHT - 1 - py : py;
      const auto color = palette.getColor(pattern.getPixel(static_cast<uint8_t>(fx), static_cast<uint8_t>(fy)));
      painter.fillRect(dx + px, dy + py, 1, 1, toQColor(color));
    }
  }
}

TileEditor::TileEditor(QWidget* parent, shared_ptr<Level>& level)
  : QDialog(parent)
  , m_level(level)
  , m_tileCombo(nullptr)
  , m_paletteCombo(nullptr)
  , m_blockList(nullptr)
  , m_hFlipCheckBox(nullptr)
  , m_vFlipCheckBox(nullptr)
  , m_canvas(nullptr)
  , m_tileIndex(0)
  , m_previewPaletteIndex(0)
  , m_dirty(false)
{
  setModal(false);

  auto* mainLayout = new QVBoxLayout();
  mainLayout->setContentsMargins(8, 8, 8, 8);
  mainLayout->setSpacing(8);
  setLayout(mainLayout);

  auto* selectorLayout = new QHBoxLayout();
  m_tileCombo = new QComboBox();
  for (size_t i = 0; i < m_level->getBlockCount(); i++) {
    m_tileCombo->addItem(QString("Tile %1").arg(i), QVariant::fromValue(i));
  }
  selectorLayout->addWidget(m_tileCombo);

  m_paletteCombo = new QComboBox();
  for (size_t i = 0; i < m_level->getPaletteCount(); i++) {
    m_paletteCombo->addItem(QString("Palette %1").arg(i), QVariant::fromValue(i));
  }
  selectorLayout->addWidget(m_paletteCombo);
  mainLayout->addLayout(selectorLayout);

  auto* editorLayout = new QHBoxLayout();
  m_canvas = new TileCanvas(this, m_level);
  editorLayout->addWidget(m_canvas);

  auto* toolsLayout = new QVBoxLayout();
  m_blockList = new QListWidget();
  m_blockList->setIconSize(QSize(Chunk::CHUNK_WIDTH * BLOCK_PREVIEW_SCALE, Chunk::CHUNK_HEIGHT * BLOCK_PREVIEW_SCALE));
  m_blockList->setMinimumWidth(170);
  populateBlockSelector();
  toolsLayout->addWidget(m_blockList);

  m_hFlipCheckBox = new QCheckBox(tr("Horizontal flip"));
  m_vFlipCheckBox = new QCheckBox(tr("Vertical flip"));
  toolsLayout->addWidget(m_hFlipCheckBox);
  toolsLayout->addWidget(m_vFlipCheckBox);
  editorLayout->addLayout(toolsLayout);
  mainLayout->addLayout(editorLayout);

  auto* buttonLayout = new QHBoxLayout();
  buttonLayout->addStretch(1);
  auto* closeButton = new QPushButton(tr("Close"));
  buttonLayout->addWidget(closeButton);
  mainLayout->addLayout(buttonLayout);

  connect(m_tileCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(tileChanged(int)));
  connect(m_paletteCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(paletteChanged(int)));
  connect(m_blockList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(blockChanged(QListWidgetItem*,QListWidgetItem*)));
  connect(m_hFlipCheckBox, SIGNAL(stateChanged(int)), this, SLOT(horizontalFlipChanged(int)));
  connect(m_vFlipCheckBox, SIGNAL(stateChanged(int)), this, SLOT(verticalFlipChanged(int)));
  connect(m_canvas, SIGNAL(tileModified()), this, SLOT(tileModified()));
  connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

  if (m_blockList->count() > 0) {
    m_blockList->setCurrentRow(0);
  }
  updateTitle();
}

QPixmap TileEditor::renderBlockPreview(size_t blockIndex, int scale) const
{
  QImage image(Chunk::CHUNK_WIDTH, Chunk::CHUNK_HEIGHT, QImage::Format_RGB888);
  image.fill(Qt::black);

  try {
    drawBlockPreview(image, m_level->getChunk(blockIndex), 0, 0);
  } catch (...) {
  }

  return QPixmap::fromImage(image.scaled(Chunk::CHUNK_WIDTH * scale,
                                         Chunk::CHUNK_HEIGHT * scale,
                                         Qt::IgnoreAspectRatio,
                                         Qt::FastTransformation));
}

void TileEditor::drawPattern(QImage& image,
                             const Pattern& pattern,
                             const Palette& palette,
                             int dx,
                             int dy,
                             bool hFlip,
                             bool vFlip) const
{
  for (int py = 0; py < Pattern::PATTERN_HEIGHT; py++) {
    for (int px = 0; px < Pattern::PATTERN_WIDTH; px++) {
      const auto fx = hFlip ? Pattern::PATTERN_WIDTH - 1 - px : px;
      const auto fy = vFlip ? Pattern::PATTERN_HEIGHT - 1 - py : py;
      const auto color = palette.getColor(pattern.getPixel(static_cast<uint8_t>(fx), static_cast<uint8_t>(fy)));
      image.setPixel(dx + px, dy + py, qRgb(color.r, color.g, color.b));
    }
  }
}

void TileEditor::drawBlockPreview(QImage& image, const Chunk& chunk, int dx, int dy) const
{
  for (int py = 0; py < 2; py++) {
    for (int px = 0; px < 2; px++) {
      const auto& patternDesc = chunk.getPatternDesc(static_cast<uint8_t>(px), static_cast<uint8_t>(py));
      const auto& pattern = m_level->getPattern(patternDesc.getPatternIndex());
      const auto& palette = m_level->getPalette(m_previewPaletteIndex);
      drawPattern(image,
                  pattern,
                  palette,
                  dx + px * Pattern::PATTERN_WIDTH,
                  dy + py * Pattern::PATTERN_HEIGHT,
                  patternDesc.getHFlip(),
                  patternDesc.getVFlip());
    }
  }
}

void TileEditor::populateBlockSelector()
{
  m_blockList->clear();
  for (size_t i = 0; i < m_level->getChunkCount(); i++) {
    auto* item = new QListWidgetItem(QIcon(renderBlockPreview(i, BLOCK_PREVIEW_SCALE)), QString("Block %1").arg(i));
    item->setData(Qt::UserRole, QVariant::fromValue(static_cast<unsigned int>(i)));
    m_blockList->addItem(item);
  }
}

void TileEditor::updateTitle()
{
  setWindowTitle(QString("%1Tile Editor - Tile %2")
      .arg(m_dirty ? "*" : "")
      .arg(m_tileIndex));
}

void TileEditor::blockChanged(QListWidgetItem* current, QListWidgetItem*)
{
  if (!current) {
    return;
  }

  m_canvas->setSelectedBlock(static_cast<uint16_t>(current->data(Qt::UserRole).toUInt()));
}

void TileEditor::horizontalFlipChanged(int state)
{
  m_canvas->setHorizontalFlip(state == Qt::Checked);
}

void TileEditor::paletteChanged(int paletteIndex)
{
  m_previewPaletteIndex = static_cast<size_t>(paletteIndex);
  m_canvas->setPreviewPalette(m_previewPaletteIndex);
  populateBlockSelector();
  if (m_blockList->count() > 0) {
    m_blockList->setCurrentRow(0);
  }
}

void TileEditor::tileChanged(int tileIndex)
{
  m_tileIndex = static_cast<size_t>(tileIndex);
  m_canvas->setTileIndex(m_tileIndex);
  updateTitle();
}

void TileEditor::tileModified()
{
  m_dirty = true;
  updateTitle();
  emit tilesModified();
}

void TileEditor::verticalFlipChanged(int state)
{
  m_canvas->setVerticalFlip(state == Qt::Checked);
}
