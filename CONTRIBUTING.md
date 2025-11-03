# Contributing to AvorionLike

Thank you for your interest in contributing to AvorionLike! This document provides guidelines and information for contributors.

## Table of Contents
1. [Getting Started](#getting-started)
2. [Development Environment](#development-environment)
3. [Code Style Guidelines](#code-style-guidelines)
4. [Project Structure](#project-structure)
5. [Making Changes](#making-changes)
6. [Testing](#testing)
7. [Submitting Changes](#submitting-changes)

## Getting Started

### Prerequisites
- Visual Studio 2022 (Community Edition or higher)
- .NET 9.0 SDK or later
- Git for version control
- Basic knowledge of C# and game development concepts

### Setting Up Your Development Environment

1. **Install Visual Studio 2022**
   - Download from: https://visualstudio.microsoft.com/vs/
   - Required workloads:
     - .NET desktop development
     - Game development with Unity (optional, for additional tools)
   - Recommended extensions:
     - GitHub Copilot (optional)
     - ReSharper (optional)
     - Visual Studio IntelliCode

2. **Install .NET 9.0 SDK**
   - Download from: https://dotnet.microsoft.com/download
   - Verify installation: `dotnet --version`

3. **Clone the Repository**
   ```bash
   git clone https://github.com/shifty81/AvorionLike.git
   cd AvorionLike
   ```

4. **Open in Visual Studio 2022**
   - Open `AvorionLike.sln` in Visual Studio 2022
   - Restore NuGet packages (should happen automatically)
   - Build the solution (Ctrl+Shift+B)

5. **Run the Application**
   - Press F5 to run with debugging
   - Or press Ctrl+F5 to run without debugging

## Development Environment

### Using Visual Studio 2022

The project is now fully configured for Visual Studio 2022:
- Solution file: `AvorionLike.sln`
- Project file: `AvorionLike/AvorionLike.csproj`
- Target framework: .NET 9.0

### Using Command Line

Alternatively, you can use the .NET CLI:
```bash
# Restore dependencies
dotnet restore

# Build
dotnet build

# Run
dotnet run --project AvorionLike

# Run tests (when available)
dotnet test
```

## Code Style Guidelines

### C# Coding Conventions

1. **Naming Conventions**
   - PascalCase for class names, method names, and properties: `GameEngine`, `CreateEntity`
   - camelCase for local variables and parameters: `entityId`, `deltaTime`
   - Prefix private fields with underscore: `_privateField`
   - Use meaningful, descriptive names

2. **Code Organization**
   - One class per file
   - File name should match class name
   - Use namespaces that reflect the folder structure
   - Order members: fields → properties → constructors → methods

3. **Comments and Documentation**
   - Use XML documentation comments for public APIs
   - Include `<summary>` tags for all public members
   - Add inline comments for complex logic
   - Keep comments up-to-date with code changes

4. **Best Practices**
   - Follow SOLID principles
   - Keep methods small and focused (ideally < 50 lines)
   - Avoid deep nesting (max 3-4 levels)
   - Use nullable reference types appropriately
   - Handle exceptions appropriately
   - Prefer composition over inheritance

### Example:

```csharp
namespace AvorionLike.Core.Systems;

/// <summary>
/// Manages entity lifecycle and component associations
/// </summary>
public class EntityManager
{
    private Dictionary<Guid, Entity> _entities = new();
    
    /// <summary>
    /// Creates a new entity with the specified name
    /// </summary>
    /// <param name="name">The name of the entity</param>
    /// <returns>The created entity instance</returns>
    public Entity CreateEntity(string name)
    {
        var entity = new Entity(name);
        _entities[entity.Id] = entity;
        return entity;
    }
}
```

## Project Structure

```
AvorionLike/
├── Core/
│   ├── ECS/              # Entity-Component System
│   ├── Voxel/            # Voxel-based architecture
│   ├── Physics/          # Physics system
│   ├── Procedural/       # Procedural generation
│   ├── Scripting/        # Lua scripting API
│   ├── Networking/       # Multiplayer networking
│   ├── Resources/        # Resource management
│   ├── RPG/              # RPG elements
│   ├── DevTools/         # Development tools
│   └── GameEngine.cs     # Main engine class
└── Program.cs            # Application entry point
```

### Adding New Features

When adding new features:
1. Place code in the appropriate Core subfolder
2. Follow the existing architectural patterns
3. Update documentation in README.md
4. Add XML documentation to all public APIs
5. Consider adding development tool support

## Making Changes

### Workflow

1. **Create a Branch**
   ```bash
   git checkout -b feature/your-feature-name
   ```

2. **Make Your Changes**
   - Write clean, well-documented code
   - Follow the code style guidelines
   - Test your changes thoroughly

3. **Commit Your Changes**
   ```bash
   git add .
   git commit -m "Add feature: brief description"
   ```
   
   Commit message format:
   - Use present tense: "Add feature" not "Added feature"
   - First line: brief summary (50 chars or less)
   - Blank line
   - Detailed description (if needed)

4. **Push Your Branch**
   ```bash
   git push origin feature/your-feature-name
   ```

### Types of Contributions

- **Bug Fixes**: Fix issues in existing code
- **New Features**: Add new systems or capabilities
- **Documentation**: Improve or expand documentation
- **Performance**: Optimize existing code
- **Tests**: Add or improve test coverage
- **Refactoring**: Improve code structure without changing behavior

## Testing

### Current Testing Approach

Currently, the project uses manual testing through the interactive console menu. When contributing:

1. **Test Your Changes**
   - Run the application and test new features
   - Test edge cases and error conditions
   - Verify existing functionality still works

2. **Document Test Steps**
   - In your pull request, describe how to test your changes
   - Include expected behavior and results

3. **Future: Automated Tests**
   - We plan to add unit tests using xUnit or NUnit
   - When added, all new features should include tests

### Testing with Development Tools

Use the built-in development tools to verify your changes:
- Press `` ` `` to open the debug console
- Use `fps` command to check performance
- Use `memory` command to check for memory leaks
- Use `profile` command for detailed performance analysis

## Submitting Changes

### Pull Request Process

1. **Ensure Your Code Builds**
   ```bash
   dotnet build
   ```

2. **Update Documentation**
   - Update README.md if you added features
   - Update CREDITS.md if you used new libraries
   - Add XML comments to new public APIs

3. **Create Pull Request**
   - Go to GitHub and create a pull request
   - Fill out the PR template (if available)
   - Link any related issues

4. **PR Description Should Include**
   - What changes were made and why
   - How to test the changes
   - Any breaking changes
   - Screenshots (for UI changes)

5. **Code Review**
   - Respond to reviewer feedback
   - Make requested changes
   - Be open to suggestions

### Review Criteria

Pull requests will be reviewed for:
- Code quality and style
- Proper documentation
- Test coverage (when available)
- No breaking changes (unless discussed)
- Performance impact
- Security considerations

## Development Tips

### Using the Development Tools

The project includes comprehensive development tools:

```csharp
// Access dev tools in GameEngine
var devTools = engine.DevTools;

// Profile a section of code
devTools.PerformanceProfiler.BeginSection("MyCode");
// ... your code ...
devTools.PerformanceProfiler.EndSection("MyCode");

// Debug rendering
devTools.DebugRenderer.DrawLine(start, end, "Red");
devTools.DebugRenderer.DrawBox(position, size, "Green");

// Console commands (press ` in running app)
// fps - Show FPS
// profile - Performance report
// memory - Memory usage
// scripts - Loaded scripts
// help - All commands
```

### Common Tasks

**Adding a New System:**
1. Create class in `Core/Systems/`
2. Inherit from `SystemBase` if it's an ECS system
3. Implement required methods
4. Register system in `GameEngine`
5. Add documentation

**Adding a New Component:**
1. Create class in appropriate Core subfolder
2. Implement `IComponent` interface
3. Add XML documentation
4. Test with existing systems

**Adding Lua Scripting Support:**
1. Register C# objects in `ScriptingEngine`
2. Document Lua API in README
3. Create example scripts

## Questions or Issues?

- Open an issue on GitHub for bugs or feature requests
- Join discussions in GitHub Discussions (if available)
- Check existing issues before creating new ones

## Code of Conduct

- Be respectful and constructive
- Focus on the code, not the person
- Welcome newcomers
- Give credit where it's due
- Follow the project's coding standards

## License

By contributing to AvorionLike, you agree that your contributions will be licensed under the MIT License.

---

Thank you for contributing to AvorionLike!
