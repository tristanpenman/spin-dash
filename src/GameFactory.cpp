#include "games/Sonic2.h"
#include "games/Sonic2Disassembly.h"
#include "games/Sonic3.h"

#include "GameFactory.h"

using namespace std;

shared_ptr<Game> GameFactory::build(const shared_ptr<Rom>& rom)
{
  // try Sonic2
  shared_ptr<Game> game = make_shared<Sonic2>(rom);
  if (game->isCompatible()) {
    return game;
  }

  // try Sonic3
  game.reset(new Sonic3(rom));
  if (game->isCompatible()) {
    return game;
  }

  return nullptr;
}

shared_ptr<Game> GameFactory::buildDisassembly(const string& iniPath)
{
  shared_ptr<Game> game = make_shared<Sonic2Disassembly>(iniPath);
  if (game->isCompatible()) {
    return game;
  }

  return nullptr;
}

shared_ptr<Game> GameFactory::buildDisassembly(const string& rootDir, const string& iniPath)
{
  shared_ptr<Game> game = make_shared<Sonic2Disassembly>(rootDir, iniPath);
  if (game->isCompatible()) {
    return game;
  }

  return nullptr;
}
