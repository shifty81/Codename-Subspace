# Codename:Subspace - Playability FAQ

**Quick answers to common questions about the current state of Codename:Subspace.**

For a detailed assessment, see [PLAYABILITY_STATUS.md](PLAYABILITY_STATUS.md) and [WHATS_LEFT_TO_IMPLEMENT.md](WHATS_LEFT_TO_IMPLEMENT.md).

---

## ❓ Is the game playable yet?

**Yes!** ✅ As of v0.9.0 (November 2025), Codename:Subspace is **playable** with a complete gameplay loop, player-controlled ships, 3D graphics, and interactive UI.

---

## ❓ What can I do with it right now?

You can:
- ✅ **Play the game!** Control your ship in 3D space
- ✅ Build custom ships with voxel blocks (9 block types, 7 material tiers)
- ✅ Mine asteroids for resources
- ✅ Trade at stations
- ✅ Combat with weapons and shields
- ✅ Explore procedural galaxy
- ✅ Manage fleet and crew
- ✅ Progress through levels
- ✅ Save/load your game
- ✅ Use 18+ different demo modes
- ✅ Write and execute Lua scripts/mods
- ✅ Use in-game testing console (~) with 40+ commands
- ✅ Access full HUD with radar, ship stats, and controls

You **cannot yet**:
- ❌ Follow structured missions/quests (not implemented)
- ❌ Get guided tutorial help (not implemented)
- ❌ Hear sound effects or music (not implemented)
- ❌ Join multiplayer servers (client UI not complete)

---

## ❓ What's the difference between "playable" and "feature-complete"?

**Playable** (what Codename:Subspace is NOW):
- Core gameplay loop works
- You can control ships and interact with the world
- All major systems functional
- Save/load works
- You can have fun playing

**Feature-Complete** (what we're working towards):
- Structured missions and objectives
- Tutorial for new players
- Sound effects and music
- Multiplayer client UI
- More content (ships, weapons, stations)
- Polish and optimization

**Analogy:** Having a functional car vs. having a luxury car with all the options.

---

## ❓ How complete is the game?

The game is **~80% complete** overall:

✅ **100% Complete:**
- Entity-Component System (ECS)
- Physics simulation (Newtonian)
- Voxel ship building (9 block types, 7 materials)
- Procedural galaxy generation
- Resource management & inventory
- Combat systems (6 weapon types)
- Mining & salvaging systems
- Trading/economy (12 trade goods)
- AI system (state-based behavior)
- Lua scripting & modding
- Multiplayer networking (server-side)
- Configuration & logging
- 3D rendering with OpenGL
- ImGui UI & HUD
- Player controls (6DOF movement)
- Save/load system
- Fleet management
- Faction system (Stellaris-style)
- Power system
- Development tools

**Assessment:** Core game is done and playable!

---

## ❓ What's missing then?

**5 major features** are not yet implemented:

❌ **Not Implemented (0-10%):**
- Quest/Mission system (structured objectives)
- Tutorial system (onboarding for new players)
- Sound/Music system (audio feedback and atmosphere)
- Steam integration (achievements, Workshop)
- Multiplayer client UI (server browser, lobby)

⚠️ **Partially Implemented (85-95%):**
- Advanced rendering (shadows, post-processing)
- Content variety (more ships, weapons, stations)

**Assessment:** Core gameplay works, needs content and polish.

---

## ❓ When will it be feature-complete?

**Current status:** ✅ PLAYABLE NOW (as of November 2025)

**Feature-complete:** 4-6 months of development remaining

**Polished Steam release:** 6-9 months total

These are estimates assuming dedicated development work.

---

## ❓ What needs to happen for feature-complete release?

**For Feature-Complete Release:**

1. **Quest/Mission System** ✨ - Structured objectives and rewards (2-3 weeks)
2. **Tutorial System** 📚 - Onboarding for new players (1-2 weeks)
3. **Sound/Music** 🔊 - Audio feedback and atmosphere (2-3 weeks)
4. **Multiplayer Client UI** 🌐 - Server browser and lobby (2-3 weeks)
5. **Content Expansion** 🚀 - More ships, weapons, stations (2-3 weeks)
6. **Polish & Optimization** ✨ - UI/UX improvements (2-3 weeks)

**Result:** Feature-complete game ready for release.

---

## ❓ Is it worth trying out?

**For Players:** Yes! ✅
- **Playable right now**
- Full gameplay experience available
- Build ships, mine, trade, and fight
- Save/load your progress
- Active development with regular updates

**For Developers:** Absolutely! ✅
- Excellent learning resource
- Great codebase to study (~35,000 lines)
- Ready for contributions
- Clean architecture
- Many features to implement

**For Modders:** Yes! ✅
- Lua scripting fully functional
- Comprehensive modding API (30+ functions)
- Auto-discovery and mod manager
- Sample mods included
- Active game to mod

---

## ❓ Can I contribute?

**Yes!** Contributions are very welcome.

**High Priority Features:**
- Quest/Mission system
- Tutorial system
- Sound/Music integration
- Multiplayer client UI
- Content creation (ships, weapons, stations)

**Skills Needed:**
- C# and .NET 9.0
- Game development experience
- UI/UX design (for tutorials and UI)
- Audio programming (for sound system)
- 3D modeling (for content creation)

See [CONTRIBUTING.md](CONTRIBUTING.md) and [WHATS_LEFT_TO_IMPLEMENT.md](WHATS_LEFT_TO_IMPLEMENT.md) for details.

---

## ❓ What should I do now?

**If you want to play:**

1. 🎮 **Download and play!** The game is ready
2. 📖 **Read [HOW_TO_BUILD_AND_RUN.md](HOW_TO_BUILD_AND_RUN.md)** for setup
3. 🕹️ **Try the full gameplay** (Option 1: NEW GAME)
4. 💡 **Provide feedback** on gameplay and features
5. 🐛 **Report bugs** if you find any

**If you're interested in contributing:**

1. ⭐ **Star the repository** on GitHub
2. 👀 **Watch the repository** for updates
3. 📖 **Read [WHATS_LEFT_TO_IMPLEMENT.md](WHATS_LEFT_TO_IMPLEMENT.md)**
4. 💻 **Pick a feature** and start coding
5. 🤝 **Join development** and submit PRs
6. 💬 **Discuss ideas** in GitHub issues

---

## ❓ Who is this for right now?

**Current Audience:**
- ✅ **Players** wanting to play a space game
- ✅ **Developers** learning game development
- ✅ **Contributors** wanting to help build features
- ✅ **Modders** creating content and mods
- ✅ **Anyone** interested in game architecture

---

## ❓ What's the project's goal?

**Mission Accomplished:** ✅
Game is now playable with full gameplay loop!

**Current goal:**
Add remaining features for feature-complete release:
- Quest/mission system
- Tutorial system
- Sound/music
- Multiplayer client UI
- Content expansion

**Long-term vision:**
Create a feature-complete game inspired by Avorion with:
- ✅ Voxel ship building (DONE)
- ✅ Open-world space exploration (DONE)
- ✅ Mining, trading, combat (DONE)
- ⚠️ Multiplayer co-op (server done, client UI needed)
- ✅ Mod support (DONE)

**Current focus:**
Adding structured content (quests, tutorial) and polish.

---

## ❓ Is this an official Avorion game?

**No.** 

Codename:Subspace is:
- Fan-made project
- Educational implementation
- Inspired by Avorion
- Not affiliated with Boxelware
- Not endorsed by the official Avorion developers

It's an independent learning project and homage to Avorion.

---

## ❓ What's the best resource to understand the current state?

**For quick overview:**
- [README.md](README.md) - Project overview
- This FAQ - Quick answers

**For detailed status:**
- [ROADMAP_STATUS.md](ROADMAP_STATUS.md) - Complete status report
- [WHATS_LEFT_TO_IMPLEMENT.md](WHATS_LEFT_TO_IMPLEMENT.md) - ⭐ **What's missing**

**For getting started:**
- [HOW_TO_BUILD_AND_RUN.md](HOW_TO_BUILD_AND_RUN.md) - Setup instructions
- [QUICKSTART.md](QUICKSTART.md) - Quick start guide

**For technical details:**
- [ARCHITECTURE.md](ARCHITECTURE.md) - System architecture
- [NEXT_STEPS.md](NEXT_STEPS.md) - Development roadmap

---

## ❓ How can I stay updated?

**Follow the project:**
- ⭐ Star on GitHub: https://github.com/shifty81/Codename-Subspace
- 👀 Watch for updates
- 📧 Subscribe to releases
- 🐛 Follow issues for development progress

**Check documentation:**
- Documents updated as project evolves
- ROADMAP_STATUS.md reflects current state
- WHATS_LEFT_TO_IMPLEMENT.md lists remaining features

---

## Summary

| Question | Answer |
|----------|--------|
| **Is it playable?** | ✅ **YES!** |
| **What is it?** | Playable space game with voxel ships |
| **Core systems complete?** | ✅ 100% |
| **Content complete?** | ⚠️ ~60% |
| **Overall complete?** | ⚠️ ~80% |
| **When feature-complete?** | 4-6 months (estimated) |
| **Can I contribute?** | ✅ Yes |
| **Should I try it?** | ✅ **Yes - play it now!** |

---

**Last Updated:** November 21, 2025

**Status:** ✅ **PLAYABLE** - Content and Polish in Progress

**Next Milestone:** Feature-Complete Release (4-6 months)

---

**What Changed Since Last Update:**
- ✅ Game is NOW PLAYABLE (v0.9.0)
- ✅ Full player controls implemented
- ✅ Interactive UI and HUD complete
- ✅ All core gameplay mechanics working
- ⚠️ 5 major features still need implementation
- ⚠️ Content expansion ongoing

**See [WHATS_LEFT_TO_IMPLEMENT.md](WHATS_LEFT_TO_IMPLEMENT.md) for complete details on remaining work.**
