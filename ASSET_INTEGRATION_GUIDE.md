# Asset Integration Guide for AvorionLike

## Overview

AvorionLike is built with **.NET 9.0 and C#**, using **Silk.NET** for OpenGL rendering. This is **NOT a Unity project**, so Unity Asset Store assets **cannot be used directly**. However, there are many other options for integrating external assets.

## Key Difference: This is NOT Unity

⚠️ **Important:** Unity Asset Store packages are designed specifically for Unity's engine and use Unity-specific formats, scripts, and APIs. They **will not work** in this custom engine without significant conversion work.

---

## Asset Types and Integration Difficulty

### ✅ EASY - Direct Integration (No Conversion Needed)

#### 1. **3D Models** (OBJ, FBX, GLTF formats)
**Difficulty:** ⭐ Easy  
**Time:** 1-2 hours per asset pipeline setup

**Where to Find:**
- [Sketchfab](https://sketchfab.com/feed) - Many free CC0 models
- [TurboSquid Free](https://www.turbosquid.com/Search/3D-Models/free/) - Free 3D models
- [OpenGameArt.org](https://opengameart.org/) - Game-ready assets
- [Poly Pizza](https://poly.pizza/) - Free low-poly models
- [Kenney.nl](https://kenney.nl/assets) - Excellent free game assets

**Integration Steps:**
```csharp
// You'll need a model loader library
// Recommended: Assimp.NET or Silk.NET.Assimp

// 1. Add NuGet package
dotnet add package AssimpNet

// 2. Create a ModelLoader class
public class ModelLoader
{
    public Mesh LoadModel(string filePath)
    {
        var importer = new AssimpContext();
        var scene = importer.ImportFile(filePath, 
            PostProcessSteps.Triangulate | 
            PostProcessSteps.GenerateNormals);
        
        // Convert Assimp mesh to your engine's mesh format
        return ConvertToEngineMesh(scene.Meshes[0]);
    }
}

// 3. Use in your renderer
var shipModel = ModelLoader.LoadModel("Assets/Models/spaceship.obj");
```

**Current Limitation:**  
The engine currently renders voxel blocks as cubes. To use 3D models:
- Extend `VoxelRenderer` to support mesh rendering
- Create a new `MeshRenderer` class
- Add mesh support to entities

---

#### 2. **Textures & Materials** (PNG, JPG, DDS formats)
**Difficulty:** ⭐ Easy  
**Time:** 30 minutes - 1 hour

**Where to Find:**
- [Texture Haven](https://polyhaven.com/textures) - CC0 PBR textures
- [CC0 Textures](https://cc0textures.com/) - Free PBR materials
- [OpenGameArt.org](https://opengameart.org/art-search-advanced?keys=&field_art_type_tid%5B%5D=10) - 2D textures
- [Kenney.nl](https://kenney.nl/assets?q=2d) - 2D sprites and UI

**Integration Steps:**
```csharp
// 1. Add texture loading with STB (already in Silk.NET)
using Silk.NET.OpenGL;
using StbImageSharp;

public class TextureLoader
{
    private GL _gl;
    
    public uint LoadTexture(string path)
    {
        // Load image
        using var stream = File.OpenRead(path);
        ImageResult image = ImageResult.FromStream(stream, ColorComponents.RedGreenBlueAlpha);
        
        // Create OpenGL texture
        uint texture = _gl.GenTexture();
        _gl.BindTexture(TextureTarget.Texture2D, texture);
        
        _gl.TexImage2D(TextureTarget.Texture2D, 0, 
            InternalFormat.Rgba, (uint)image.Width, (uint)image.Height, 0,
            PixelFormat.Rgba, PixelType.UnsignedByte, image.Data);
        
        _gl.GenerateMipmap(TextureTarget.Texture2D);
        return texture;
    }
}

// 2. Update shader to use textures
// In your vertex shader:
// layout(location = 2) in vec2 aTexCoord;
// out vec2 TexCoord;

// In fragment shader:
// uniform sampler2D uTexture;
// color = texture(uTexture, TexCoord);
```

---

#### 3. **Audio Files** (WAV, OGG, MP3 formats)
**Difficulty:** ⭐⭐ Moderate  
**Time:** 2-3 hours for audio system setup

**Where to Find:**
- [Freesound.org](https://freesound.org/) - Thousands of free sounds
- [OpenGameArt.org](https://opengameart.org/art-search-advanced?keys=&field_art_type_tid%5B%5D=13) - Game audio
- [Incompetech](https://incompetech.com/music/) - Free music by Kevin MacLeod
- [Kenney.nl](https://kenney.nl/assets?q=audio) - Sound effects

**Integration Steps:**
```csharp
// 1. Add OpenAL.NET for audio (Silk.NET includes it)
dotnet add package Silk.NET.OpenAL

// 2. Create AudioSystem
using Silk.NET.OpenAL;

public class AudioSystem
{
    private ALContext _alContext;
    private ALDevice _alDevice;
    private uint _source;
    
    public void Initialize()
    {
        _alContext = ALContext.GetApi(true);
        _alDevice = ALDevice.GetApi(true);
        // Setup audio source
        _source = _alContext.GenSource();
    }
    
    public void PlaySound(string filePath)
    {
        // Load audio file (using NVorbis for OGG, or NAudio for MP3/WAV)
        uint buffer = LoadAudioBuffer(filePath);
        _alContext.SetSourceProperty(_source, SourceInteger.Buffer, buffer);
        _alContext.SourcePlay(_source);
    }
}

// 3. Add to GameEngine
public class GameEngine
{
    public AudioSystem AudioSystem { get; private set; } = null!;
    
    private void Initialize()
    {
        // ... existing code ...
        AudioSystem = new AudioSystem();
        AudioSystem.Initialize();
    }
}
```

**Recommended Audio Libraries:**
- **NVorbis** - For OGG files
- **NAudio** - For MP3/WAV files
- **Silk.NET.OpenAL** - For 3D positional audio

---

### ⚠️ MODERATE - Requires Conversion

#### 4. **Unity Models & Scenes**
**Difficulty:** ⭐⭐⭐ Hard  
**Time:** 4-8 hours per asset + significant rework

**The Problem:**
- Unity uses proprietary formats (`.unity`, `.prefab`, `.asset`)
- Unity scripts are MonoBehaviour-based (incompatible)
- Unity shaders are ShaderLab (need conversion to GLSL)

**Possible Solutions:**

**Option A: Export from Unity**
1. Open Unity project with the asset
2. Export models as FBX/OBJ: `Select model → Export → FBX`
3. Export textures: Right-click → Show in Explorer → Copy
4. Manually recreate scripts in C#

**Option B: Use Asset Ripper**
- [AssetRipper](https://github.com/AssetRipper/AssetRipper) - Extract Unity assets
- Exports to standard formats (FBX, PNG, etc.)
- Loses Unity-specific functionality

```bash
# Using AssetRipper
1. Download AssetRipper from GitHub
2. Load Unity asset bundle or project
3. Export as FBX + textures
4. Import into AvorionLike using Assimp
```

---

#### 5. **Shaders**
**Difficulty:** ⭐⭐⭐⭐ Very Hard  
**Time:** 8-16 hours per complex shader

**Unity ShaderLab → GLSL Conversion:**

Unity ShaderLab is a high-level language that compiles to multiple backends. You'll need to:

1. **Manual Conversion:**
```glsl
// Unity ShaderLab (CAN'T USE DIRECTLY)
Shader "Custom/MyShader" {
    Properties {
        _MainTex ("Texture", 2D) = "white" {}
        _Color ("Color", Color) = (1,1,1,1)
    }
    // ... Unity-specific code
}

// Convert to GLSL for OpenGL (WHAT YOU NEED)
// Vertex Shader (vertex.glsl)
#version 330 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;

out vec2 TexCoord;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main()
{
    gl_Position = uProjection * uView * uModel * vec4(aPosition, 1.0);
    TexCoord = aTexCoord;
}

// Fragment Shader (fragment.glsl)
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D uMainTex;
uniform vec4 uColor;

void main()
{
    FragColor = texture(uMainTex, TexCoord) * uColor;
}
```

2. **Tools to Help:**
- [Shader Playground](http://shader-playground.timjones.io/) - Shader conversion
- Study GLSL tutorials: [LearnOpenGL.com](https://learnopengl.com/Getting-started/Shaders)

---

### ✅ EASY - Native C# Assets

#### 6. **C# Libraries & Packages**
**Difficulty:** ⭐ Easy  
**Time:** 30 minutes - 2 hours

**Recommended NuGet Packages:**

```bash
# Physics
dotnet add package BepuPhysics2  # Advanced physics engine

# Networking
dotnet add package LiteNetLib     # Lightweight networking
dotnet add package Steamworks.NET # Steam integration

# Math & Utilities
dotnet add package MathNet.Numerics
dotnet add package MessagePack    # Fast serialization

# Model Loading
dotnet add package AssimpNet      # 3D model loading
dotnet add package Silk.NET.Assimp

# Audio
dotnet add package NVorbis        # OGG audio
dotnet add package NAudio         # Audio playback

# Image Processing
dotnet add package SixLabors.ImageSharp
dotnet add package StbImageSharp  # Already in Silk.NET

# UI
dotnet add package ImGui.NET      # Already included
dotnet add package Myra           # Alternative UI framework
```

---

## Recommended Asset Workflow

### Step 1: Asset Preparation
```
Project Structure:
AvorionLike/
├── AvorionLike/
│   └── Assets/
│       ├── Models/          # FBX, OBJ, GLTF files
│       ├── Textures/        # PNG, JPG, DDS files
│       ├── Audio/
│       │   ├── Music/       # OGG, MP3 files
│       │   └── SFX/         # WAV, OGG files
│       ├── Shaders/         # GLSL shader files
│       └── Data/            # JSON, XML game data
```

### Step 2: Create Asset Management System

```csharp
// New file: AvorionLike/Core/Assets/AssetManager.cs
public class AssetManager
{
    private Dictionary<string, Mesh> _meshCache = new();
    private Dictionary<string, uint> _textureCache = new();
    private Dictionary<string, uint> _audioCache = new();
    
    public Mesh LoadModel(string path)
    {
        if (_meshCache.TryGetValue(path, out var cached))
            return cached;
            
        var mesh = ModelLoader.LoadModel(path);
        _meshCache[path] = mesh;
        return mesh;
    }
    
    public uint LoadTexture(string path)
    {
        if (_textureCache.TryGetValue(path, out var cached))
            return cached;
            
        var texture = TextureLoader.LoadTexture(path);
        _textureCache[path] = texture;
        return texture;
    }
    
    public void UnloadAll()
    {
        _meshCache.Clear();
        _textureCache.Clear();
        _audioCache.Clear();
    }
}
```

### Step 3: Integrate into Game Engine

```csharp
// In GameEngine.cs
public class GameEngine
{
    public AssetManager AssetManager { get; private set; } = null!;
    
    private void Initialize()
    {
        // ... existing initialization ...
        AssetManager = new AssetManager();
        Logger.Instance.Info("GameEngine", "AssetManager initialized");
    }
}
```

---

## Best Free Asset Sources (Non-Unity)

### 3D Models
1. **[Kenney.nl](https://kenney.nl/assets)** - ⭐⭐⭐⭐⭐
   - CC0 Licensed (Public Domain)
   - Space ships, planets, asteroids
   - Perfect for this project!

2. **[OpenGameArt.org](https://opengameart.org/)** - ⭐⭐⭐⭐
   - Various licenses (check each)
   - Large community
   - Game-ready assets

3. **[Sketchfab](https://sketchfab.com/features/gltf)** - ⭐⭐⭐⭐
   - Many CC-BY or CC0 models
   - Download as GLTF/GLB
   - High quality

4. **[Poly Pizza](https://poly.pizza/)** - ⭐⭐⭐
   - CC0 Licensed
   - Former Google Poly archive
   - Low-poly style

### Textures & Materials
1. **[Poly Haven](https://polyhaven.com/)** - ⭐⭐⭐⭐⭐
   - CC0 PBR textures
   - HDRIs for lighting
   - Professional quality

2. **[CC0 Textures](https://cc0textures.com/)** - ⭐⭐⭐⭐⭐
   - 100% free, CC0
   - PBR workflows
   - 4K textures

### Audio
1. **[Freesound.org](https://freesound.org/)** - ⭐⭐⭐⭐⭐
   - Massive library
   - Various CC licenses
   - Community-driven

2. **[OpenGameArt.org Music](https://opengameart.org/art-search-advanced?field_art_type_tid=13)** - ⭐⭐⭐⭐
   - Game-specific music
   - Various licenses

---

## Implementation Priority

### Phase 1: Basic Asset Support (Week 1-2)
- [x] ✅ Already have: Basic voxel rendering
- [ ] 🎯 Add texture support to existing renderer
- [ ] 🎯 Create TextureLoader class
- [ ] 🎯 Update shaders for texture mapping
- [ ] 🎯 Test with Kenney's texture packs

### Phase 2: 3D Model Support (Week 3-4)
- [ ] Add AssimpNet NuGet package
- [ ] Create ModelLoader class
- [ ] Create Mesh data structure
- [ ] Create MeshRenderer (separate from VoxelRenderer)
- [ ] Add model caching in AssetManager
- [ ] Test with simple spaceship models

### Phase 3: Audio System (Week 5-6)
- [ ] Add Silk.NET.OpenAL
- [ ] Add NVorbis for OGG support
- [ ] Create AudioSystem class
- [ ] Implement 3D positional audio
- [ ] Add background music system
- [ ] Test with sound effects

### Phase 4: Advanced Features (Week 7-8)
- [ ] Particle system for explosions
- [ ] Skybox/space background
- [ ] Post-processing effects
- [ ] LOD (Level of Detail) system
- [ ] Animation support (for turrets, engines)

---

## Example: Quick Start with Kenney Assets

Kenney.nl has **perfect** assets for this project!

### 1. Download Kenney Space Kit
```bash
# Go to: https://kenney.nl/assets/space-kit
# Download the package (it's free!)
# Extract to: AvorionLike/Assets/Models/SpaceKit/
```

### 2. Create Model Loading Test
```csharp
// In Program.cs, add a new demo option
case "14":
    ModelLoadingDemo();
    break;

static void ModelLoadingDemo()
{
    Console.WriteLine("\n=== Model Loading Demo ===");
    
    // This will require implementing ModelLoader first
    var assetManager = new AssetManager();
    var shipMesh = assetManager.LoadModel("Assets/Models/SpaceKit/ship_01.obj");
    
    Console.WriteLine($"Loaded model with {shipMesh.VertexCount} vertices");
}
```

### 3. Render in 3D Window
```csharp
// Extend VoxelRenderer or create new MeshRenderer
public class MeshRenderer
{
    private GL _gl;
    private uint _vao, _vbo, _ebo;
    
    public void LoadMesh(Mesh mesh)
    {
        // Setup VAO, VBO, EBO with mesh data
        // Similar to VoxelRenderer but with mesh vertices
    }
    
    public void Render(Mesh mesh, Matrix4x4 transform)
    {
        // Render the mesh
    }
}
```

---

## TL;DR - Quick Answer

### Can you use Unity Asset Store assets?
**❌ NO - Not directly.** Unity assets are Unity-specific and won't work in this custom engine.

### What CAN you use?
**✅ YES:**
- 3D models (OBJ, FBX, GLTF) - **EASY**
- Textures (PNG, JPG) - **EASY**
- Audio files (OGG, MP3, WAV) - **MODERATE**
- C# libraries from NuGet - **EASY**

**⚠️ WITH EFFORT:**
- Unity models (export as FBX first) - **HARD**
- Unity shaders (convert to GLSL) - **VERY HARD**

### Recommended FREE Sources:
1. **[Kenney.nl](https://kenney.nl/)** - Best for this project! Space assets, CC0
2. **[OpenGameArt.org](https://opengameart.org/)** - Large variety
3. **[Poly Haven](https://polyhaven.com/)** - Textures and HDRIs
4. **[Sketchfab](https://sketchfab.com/)** - 3D models (CC-BY/CC0)

### Time Estimate for Asset Integration:
- **Texture support:** 2-3 hours
- **3D model loading:** 1-2 days
- **Audio system:** 2-3 days
- **Full asset pipeline:** 1-2 weeks

---

## Next Steps

Want me to implement any of these asset systems? I can:
1. ✅ Add texture support to the renderer (2-3 hours)
2. ✅ Implement 3D model loading with AssimpNet (1-2 days)
3. ✅ Create audio system with OpenAL (2-3 days)
4. ✅ Set up complete asset management (1 week)

Let me know what you'd like to prioritize!
