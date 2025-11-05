# AvorionLike - Playability FAQ

**Quick answers to common questions about the current state of AvorionLike.**

For a detailed assessment, see [PLAYABILITY_STATUS.md](PLAYABILITY_STATUS.md).

---

## ❓ Is the game playable yet?

**No.** AvorionLike is currently a **game engine** with technology demonstrations, not a playable game.

---

## ❓ What can I do with it right now?

You can:
- ✅ Run 10 different system demonstrations
- ✅ View 3D voxel ships in a renderer
- ✅ Write and execute Lua scripts/mods
- ✅ Explore the codebase and architecture
- ✅ Contribute to development

You **cannot**:
- ❌ Play a game with objectives
- ❌ Control a ship interactively
- ❌ Mine, trade, or fight in real-time
- ❌ Experience a gameplay loop
- ❌ Save and continue a game session

---

## ❓ What's the difference between a "game engine" and a "game"?

**Game Engine** (what AvorionLike currently is):
- Backend systems that power a game
- Physics, rendering, networking, etc.
- Technical demonstrations
- Foundation for building games

**Game** (what AvorionLike is working towards):
- Interactive experience with objectives
- Player makes choices and progresses
- Win/lose conditions
- Fun to play

**Analogy:** Having a car engine vs. having a drivable car.

---

## ❓ How complete is the backend?

The backend is **95% complete** and production-ready:

✅ **Fully Implemented:**
- Entity-Component System
- Physics simulation
- Voxel ship building
- Procedural galaxy generation
- Resource management
- Combat systems
- Mining systems
- Trading/economy
- Lua scripting
- Multiplayer networking (server)
- Configuration & logging
- 3D rendering

**Assessment:** The hard part is done!

---

## ❓ What's missing then?

The **gameplay layer** is missing (15% complete):

❌ **Not Implemented:**
- Game loop (continuous play)
- Player ship controls
- Interactive UI/HUD
- Real-time interactions (mining, trading, combat)
- Objectives/missions
- AI opponents
- Game state management
- Multiplayer client

**Assessment:** Systems exist but aren't wired together for gameplay.

---

## ❓ When will it be playable?

**Minimum playable:** 4-6 weeks of focused development

**Feature complete:** 5-7 months of development

**Polished release:** Add 2-3 additional months

These are estimates assuming dedicated development work.

---

## ❓ What needs to happen for it to be playable?

**Minimum Viable Playable Game (MVP):**

1. **Game loop** - Continuous update/render cycle
2. **Player controls** - Control ship with keyboard/mouse
3. **Basic HUD** - Show health, shields, speed, etc.
4. **Interactive systems** - Mine, trade, jump
5. **Starting scenario** - Spawn player with basic ship
6. **Basic objectives** - Simple goals to work towards

**Result:** A simple but playable game experience.

---

## ❓ Is it worth trying out?

**For Players:** Not yet
- Wait 4-6 weeks minimum
- Currently only demos to watch

**For Developers:** Yes!
- Excellent learning resource
- Great codebase to study
- Ready for contributions
- Clean architecture

**For Modders:** Somewhat
- Lua scripting works
- Can write mods
- No game to mod yet
- Good for preparing

---

## ❓ Can I contribute?

**Yes!** Contributions are welcome.

**High Priority:**
- Game loop implementation
- Player input system
- HUD/UI framework
- Interactive system integration
- AI opponent system

**Skills needed:**
- C# and .NET
- Game development experience
- UI/UX design (for HUD)

See [CONTRIBUTING.md](CONTRIBUTING.md) for details.

---

## ❓ What should I do while waiting for playability?

**If you're interested in the project:**

1. ⭐ **Star the repository** on GitHub
2. 👀 **Watch the repository** for updates
3. 📖 **Read the documentation** to understand the systems
4. 💻 **Try the demos** to see what's implemented
5. 🤝 **Join development** if you're a developer
6. 💡 **Provide feedback** on the direction

**If you just want to play:**
- Check back in 4-6 weeks
- We'll announce when it's playable

---

## ❓ Who is this for right now?

**Current Audience:**
- ✅ Developers learning game development
- ✅ Contributors wanting to help
- ✅ People interested in game architecture
- ✅ Modders preparing for future
- ❌ Players wanting to play a game *(not yet)*

---

## ❓ What's the project's goal?

**Short-term goal:**
Make AvorionLike playable within 4-6 weeks

**Long-term goal:**
Create a feature-complete game inspired by Avorion with:
- Voxel ship building
- Open-world space exploration
- Mining, trading, combat
- Multiplayer co-op
- Mod support

**Current focus:**
Implementing the gameplay layer to make it playable.

---

## ❓ Is this an official Avorion game?

**No.** 

AvorionLike is:
- Fan-made project
- Educational implementation
- Inspired by Avorion
- Not affiliated with Boxelware
- Not endorsed by the official Avorion developers

It's a learning project and homage to Avorion.

---

## ❓ What's the best resource to understand the current state?

**For quick overview:**
- [README.md](README.md) - Project overview
- [PROJECT_SUMMARY.md](PROJECT_SUMMARY.md) - Current state summary

**For detailed assessment:**
- [PLAYABILITY_STATUS.md](PLAYABILITY_STATUS.md) - Complete playability analysis

**For technical details:**
- [ARCHITECTURE.md](ARCHITECTURE.md) - System architecture
- [FEATURES.md](FEATURES.md) - Feature documentation

**For next steps:**
- [NEXT_STEPS.md](NEXT_STEPS.md) - Development roadmap

---

## ❓ How can I stay updated?

**Follow the project:**
- ⭐ Star on GitHub: https://github.com/shifty81/AvorionLike
- 👀 Watch for updates
- 📧 Subscribe to releases

**Check documentation:**
- Documents updated as project evolves
- PLAYABILITY_STATUS.md will reflect current state

---

## Summary

| Question | Answer |
|----------|--------|
| **Is it playable?** | ❌ No |
| **What is it?** | Game engine with demos |
| **Backend complete?** | ✅ 95% |
| **Frontend complete?** | ⚠️ 15% |
| **When playable?** | 4-6 weeks (estimated) |
| **Can I contribute?** | ✅ Yes |
| **Should I try it?** | Devs: ✅ Yes, Players: ⏳ Wait |

---

**Last Updated:** November 5, 2025

**Status:** Not Playable - Development in Progress

**Next Milestone:** Minimum Viable Playable Game (4-6 weeks)
