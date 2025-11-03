# Quick Start Guide - AvorionLike

This guide will help you get AvorionLike up and running in minutes!

## 🚀 One-Click Setup

### Windows Users
1. Open PowerShell in the project directory
2. Run:
   ```powershell
   .\setup.ps1
   ```
3. Wait for the script to complete
4. Start the application with: `cd AvorionLike` then `dotnet run`

### Linux/macOS Users
1. Open Terminal in the project directory
2. Run:
   ```bash
   ./setup.sh
   ```
3. Wait for the script to complete
4. Start the application with: `cd AvorionLike` then `dotnet run`

## ✅ Prerequisites Check

Not sure if your system is ready? Run the prerequisites checker:

**Windows:**
```powershell
.\check-prerequisites.ps1
```

**Linux/macOS:**
```bash
./check-prerequisites.sh
```

This will tell you exactly what's missing (if anything).

## 📋 What You Need

- **.NET 9.0 SDK or later**
  - Download: https://dotnet.microsoft.com/download
  - Check if installed: `dotnet --version`

That's it! The setup scripts handle everything else automatically.

## 🎮 Running the Application

After setup is complete:

```bash
cd AvorionLike
dotnet run
```

You'll see an interactive menu with various demos:
- Engine Demo
- Voxel System Demo
- Physics Demo
- Procedural Generation
- And more!

## 🔧 Manual Setup (Advanced)

If you prefer manual control:

```bash
# 1. Navigate to project
cd AvorionLike

# 2. Restore dependencies
dotnet restore

# 3. Build
dotnet build

# 4. Run
dotnet run
```

## ❓ Need Help?

### Common Issues

**"dotnet command not found"**
- Install .NET SDK from: https://dotnet.microsoft.com/download
- Restart your terminal after installation

**"Script execution disabled" (Windows)**
- Run PowerShell as Administrator
- Execute: `Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser`

**"Permission denied" (Linux/macOS)**
- Make scripts executable: `chmod +x setup.sh check-prerequisites.sh`

### Still Stuck?
Check the [full README](README.md) for detailed troubleshooting or open an issue on GitHub.

## 🎯 Next Steps

Once the application is running:
1. Try the Engine Demo (option 1) to create a test ship
2. Explore the Voxel System Demo (option 2) to build structures
3. Check out the Scripting Demo (option 7) to see Lua in action
4. Read the [full documentation](README.md) for detailed API usage

Happy coding! 🚀
