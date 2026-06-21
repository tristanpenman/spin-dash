#pragma once

#include <memory>
#include <vector>

#include <QDialog>
#include <QWidget>

class QCheckBox;
class QComboBox;
class QLabel;
class QListWidget;
class QListWidgetItem;

class Block;
class Chunk;
class Level;
class Palette;
class Pattern;

class TileCanvas : public QWidget
{
  Q_OBJECT

public:
  TileCanvas(QWidget* parent, std::shared_ptr<Level>& level);

  void setTileIndex(size_t tileIndex);
  void setPreviewPalette(size_t paletteIndex);
  void setSelectedBlock(uint16_t blockIndex);
  void setHorizontalFlip(bool enabled);
  void setVerticalFlip(bool enabled);

protected:
  void mouseMoveEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void paintEvent(QPaintEvent* event) override;

private:
  void drawAt(const QPoint& pos);
  void drawBlock(QPainter& painter, const Block& block);
  void drawChunk(QPainter& painter, const Chunk& chunk, int dx, int dy, bool hFlip, bool vFlip);
  void drawPattern(QPainter& painter, const Pattern& pattern, const Palette& palette, int dx, int dy, bool hFlip, bool vFlip);

  std::shared_ptr<Level> m_level;
  size_t m_tileIndex;
  size_t m_previewPaletteIndex;
  uint16_t m_selectedBlockIndex;
  bool m_hFlip;
  bool m_vFlip;
  int m_highlightX;
  int m_highlightY;

signals:
  void tileModified();
};

class TileEditor : public QDialog
{
  Q_OBJECT

public:
  TileEditor(QWidget* parent, std::shared_ptr<Level>& level);

private:
  QPixmap renderBlockPreview(size_t blockIndex, int scale) const;
  void drawPattern(QImage& image, const Pattern& pattern, const Palette& palette, int dx, int dy, bool hFlip, bool vFlip) const;
  void drawBlockPreview(QImage& image, const Chunk& chunk, int dx, int dy) const;
  void populateBlockSelector();
  void updateTitle();

  std::shared_ptr<Level> m_level;

  QComboBox* m_tileCombo;
  QComboBox* m_paletteCombo;
  QListWidget* m_blockList;
  QCheckBox* m_hFlipCheckBox;
  QCheckBox* m_vFlipCheckBox;
  TileCanvas* m_canvas;

  size_t m_tileIndex;
  size_t m_previewPaletteIndex;
  bool m_dirty;

private slots:
  void blockChanged(QListWidgetItem* current, QListWidgetItem* previous);
  void horizontalFlipChanged(int state);
  void paletteChanged(int paletteIndex);
  void tileChanged(int tileIndex);
  void tileModified();
  void verticalFlipChanged(int state);

signals:
  void tilesModified();
};
