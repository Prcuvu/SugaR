/*
  SugaR, a UCI chess playing engine derived from Stockfish
  Copyright (C) 2008-2015 Marco Costalba, Joona Kiiski, Tord Romstad

  SugaR is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  SugaR is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef THREAD_H_INCLUDED
#define THREAD_H_INCLUDED

#include <atomic>
#include <bitset>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

#include "material.h"
#include "movepick.h"
#include "pawns.h"
#include "position.h"
#include "search.h"
#include "thread_win32.h"


/// Thread struct keeps together all the thread related stuff. We also use
/// per-thread pawn and material hash tables so that once we get a pointer to an
/// entry its life time is unlimited and we don't have to care about someone
/// changing the entry under our feet.

struct Thread : public std::thread {

  Thread();
  virtual ~Thread();
  virtual void search();
  void idle_loop();
  void join();
  void notify_one();
  void wait(std::atomic_bool& b);

  std::atomic_bool exit, searching, resetCalls;
  Mutex mutex;
  ConditionVariable sleepCondition;

  Pawns::Table pawnsTable;
  Material::Table materialTable;
  Endgames endgames;
  size_t idx, PVIdx;
  int maxPly, callsCnt;

  Position rootPos;
  Search::RootMoveVector rootMoves;
  Depth rootDepth;
  HistoryStats history;
  MovesStats counterMoves;
  Depth completedDepth;
};


/// MainThread is a derived classes used to characterize the the main one

struct MainThread : public Thread {
  virtual void search();
};


/// ThreadPool struct handles all the threads related stuff like init, starting,
/// parking and, most importantly, launching a thread.
/// All the access to shared thread data is done through this class.

struct ThreadPool : public std::vector<Thread*> {

  void init(); // No constructor and destructor, threads rely on globals that should
  void exit(); // be initialized and valid during the whole thread lifetime.

  MainThread* main() { return static_cast<MainThread*>(at(0)); }
  void read_uci_options();
  void start_thinking(const Position&, const Search::LimitsType&, Search::StateStackPtr&);
  int64_t nodes_searched();
};

extern ThreadPool Threads;

#endif // #ifndef THREAD_H_INCLUDED
