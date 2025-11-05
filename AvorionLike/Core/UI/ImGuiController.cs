using ImGuiNET;
using Silk.NET.OpenGL;
using Silk.NET.Input;
using Silk.NET.Windowing;
using System.Numerics;
using System.Runtime.CompilerServices;

namespace AvorionLike.Core.UI;

/// <summary>
/// ImGui controller for Silk.NET integration
/// Handles ImGui rendering and input processing
/// </summary>
public class ImGuiController : IDisposable
{
    private GL _gl;
    private IWindow _window;
    private IInputContext _input;
    
    private uint _vertexArray;
    private uint _vertexBuffer;
    private uint _elementBuffer;
    private uint _shaderProgram;
    private uint _fontTexture;
    
    private int _attribLocationTex;
    private int _attribLocationProjMtx;
    private int _attribLocationVtxPos;
    private int _attribLocationVtxUV;
    private int _attribLocationVtxColor;
    
    private int _windowWidth;
    private int _windowHeight;
    
    private bool _frameBegun;
    private bool _disposed;
    
    private readonly List<char> _pressedChars = new();
    
    public ImGuiController(GL gl, IWindow window, IInputContext input)
    {
        _gl = gl;
        _window = window;
        _input = input;
        
        _windowWidth = window.Size.X;
        _windowHeight = window.Size.Y;
        
        // Initialize ImGui
        IntPtr context = ImGui.CreateContext();
        ImGui.SetCurrentContext(context);
        
        var io = ImGui.GetIO();
        io.BackendFlags |= ImGuiBackendFlags.RendererHasVtxOffset;
        io.ConfigFlags |= ImGuiConfigFlags.NavEnableKeyboard;
        
        // Setup display size
        io.DisplaySize = new Vector2(_windowWidth, _windowHeight);
        io.DisplayFramebufferScale = Vector2.One;
        
        // Create device objects
        CreateDeviceResources();
        
        // Setup key mapping
        SetupKeyMap(io);
        
        // Hook up input events
        SetupInput();
        
        ImGui.StyleColorsDark();
    }
    
    private void SetupKeyMap(ImGuiIOPtr io)
    {
        // ImGui 1.91+ doesn't use KeyMap array anymore, it uses the new io.AddKeyEvent API
        // We'll handle key events in the OnKeyDown/OnKeyUp methods directly
    }
    
    private void SetupInput()
    {
        // Keyboard input
        foreach (var keyboard in _input.Keyboards)
        {
            keyboard.KeyDown += OnKeyDown;
            keyboard.KeyUp += OnKeyUp;
            keyboard.KeyChar += OnKeyChar;
        }
        
        // Mouse input
        foreach (var mouse in _input.Mice)
        {
            mouse.MouseDown += OnMouseDown;
            mouse.MouseUp += OnMouseUp;
            mouse.Scroll += OnScroll;
        }
    }
    
    private void OnKeyDown(IKeyboard keyboard, Key key, int code)
    {
        var io = ImGui.GetIO();
        var imguiKey = TranslateKey(key);
        if (imguiKey != ImGuiKey.None)
        {
            io.AddKeyEvent(imguiKey, true);
        }
        UpdateModifiers(keyboard);
    }
    
    private void OnKeyUp(IKeyboard keyboard, Key key, int code)
    {
        var io = ImGui.GetIO();
        var imguiKey = TranslateKey(key);
        if (imguiKey != ImGuiKey.None)
        {
            io.AddKeyEvent(imguiKey, false);
        }
        UpdateModifiers(keyboard);
    }
    
    private ImGuiKey TranslateKey(Key key)
    {
        return key switch
        {
            Key.Tab => ImGuiKey.Tab,
            Key.Left => ImGuiKey.LeftArrow,
            Key.Right => ImGuiKey.RightArrow,
            Key.Up => ImGuiKey.UpArrow,
            Key.Down => ImGuiKey.DownArrow,
            Key.PageUp => ImGuiKey.PageUp,
            Key.PageDown => ImGuiKey.PageDown,
            Key.Home => ImGuiKey.Home,
            Key.End => ImGuiKey.End,
            Key.Delete => ImGuiKey.Delete,
            Key.Backspace => ImGuiKey.Backspace,
            Key.Enter => ImGuiKey.Enter,
            Key.Escape => ImGuiKey.Escape,
            Key.Space => ImGuiKey.Space,
            Key.A => ImGuiKey.A,
            Key.B => ImGuiKey.B,
            Key.C => ImGuiKey.C,
            Key.D => ImGuiKey.D,
            Key.E => ImGuiKey.E,
            Key.F => ImGuiKey.F,
            Key.G => ImGuiKey.G,
            Key.H => ImGuiKey.H,
            Key.I => ImGuiKey.I,
            Key.J => ImGuiKey.J,
            Key.K => ImGuiKey.K,
            Key.L => ImGuiKey.L,
            Key.M => ImGuiKey.M,
            Key.N => ImGuiKey.N,
            Key.O => ImGuiKey.O,
            Key.P => ImGuiKey.P,
            Key.Q => ImGuiKey.Q,
            Key.R => ImGuiKey.R,
            Key.S => ImGuiKey.S,
            Key.T => ImGuiKey.T,
            Key.U => ImGuiKey.U,
            Key.V => ImGuiKey.V,
            Key.W => ImGuiKey.W,
            Key.X => ImGuiKey.X,
            Key.Y => ImGuiKey.Y,
            Key.Z => ImGuiKey.Z,
            Key.F1 => ImGuiKey.F1,
            Key.F2 => ImGuiKey.F2,
            Key.F3 => ImGuiKey.F3,
            Key.F4 => ImGuiKey.F4,
            Key.F5 => ImGuiKey.F5,
            Key.F6 => ImGuiKey.F6,
            Key.F7 => ImGuiKey.F7,
            Key.F8 => ImGuiKey.F8,
            Key.F9 => ImGuiKey.F9,
            Key.F10 => ImGuiKey.F10,
            Key.F11 => ImGuiKey.F11,
            Key.F12 => ImGuiKey.F12,
            _ => ImGuiKey.None
        };
    }
    
    private void OnKeyChar(IKeyboard keyboard, char c)
    {
        _pressedChars.Add(c);
    }
    
    private void OnMouseDown(IMouse mouse, MouseButton button)
    {
        var io = ImGui.GetIO();
        if (button == MouseButton.Left) io.MouseDown[0] = true;
        if (button == MouseButton.Right) io.MouseDown[1] = true;
        if (button == MouseButton.Middle) io.MouseDown[2] = true;
    }
    
    private void OnMouseUp(IMouse mouse, MouseButton button)
    {
        var io = ImGui.GetIO();
        if (button == MouseButton.Left) io.MouseDown[0] = false;
        if (button == MouseButton.Right) io.MouseDown[1] = false;
        if (button == MouseButton.Middle) io.MouseDown[2] = false;
    }
    
    private void OnScroll(IMouse mouse, ScrollWheel scrollWheel)
    {
        var io = ImGui.GetIO();
        io.MouseWheel = scrollWheel.Y;
        io.MouseWheelH = scrollWheel.X;
    }
    
    private void UpdateModifiers(IKeyboard keyboard)
    {
        var io = ImGui.GetIO();
        io.KeyCtrl = keyboard.IsKeyPressed(Key.ControlLeft) || keyboard.IsKeyPressed(Key.ControlRight);
        io.KeyShift = keyboard.IsKeyPressed(Key.ShiftLeft) || keyboard.IsKeyPressed(Key.ShiftRight);
        io.KeyAlt = keyboard.IsKeyPressed(Key.AltLeft) || keyboard.IsKeyPressed(Key.AltRight);
        io.KeySuper = keyboard.IsKeyPressed(Key.SuperLeft) || keyboard.IsKeyPressed(Key.SuperRight);
    }
    
    public void Update(float deltaTime)
    {
        if (_frameBegun)
        {
            ImGui.Render();
        }
        
        var io = ImGui.GetIO();
        
        // Update display size (handles window resize)
        _windowWidth = _window.Size.X;
        _windowHeight = _window.Size.Y;
        io.DisplaySize = new Vector2(_windowWidth, _windowHeight);
        io.DisplayFramebufferScale = Vector2.One;
        io.DeltaTime = deltaTime;
        
        // Update mouse position
        if (_input.Mice.Count > 0)
        {
            var mouse = _input.Mice[0];
            io.MousePos = new Vector2(mouse.Position.X, mouse.Position.Y);
        }
        
        // Process text input
        foreach (var c in _pressedChars)
        {
            io.AddInputCharacter(c);
        }
        _pressedChars.Clear();
        
        _frameBegun = true;
        ImGui.NewFrame();
    }
    
    public void Render()
    {
        if (!_frameBegun)
            return;
        
        _frameBegun = false;
        ImGui.Render();
        RenderImDrawData(ImGui.GetDrawData());
    }
    
    private void CreateDeviceResources()
    {
        // Create buffers
        _vertexArray = _gl.GenVertexArray();
        _vertexBuffer = _gl.GenBuffer();
        _elementBuffer = _gl.GenBuffer();
        
        // Create shader
        CreateShader();
        
        // Create font texture
        CreateFontTexture();
    }
    
    private void CreateShader()
    {
        string vertexShader = @"
            #version 330 core
            layout (location = 0) in vec2 Position;
            layout (location = 1) in vec2 UV;
            layout (location = 2) in vec4 Color;
            
            uniform mat4 ProjMtx;
            
            out vec2 Frag_UV;
            out vec4 Frag_Color;
            
            void main()
            {
                Frag_UV = UV;
                Frag_Color = Color;
                gl_Position = ProjMtx * vec4(Position.xy, 0, 1);
            }
        ";
        
        string fragmentShader = @"
            #version 330 core
            in vec2 Frag_UV;
            in vec4 Frag_Color;
            
            uniform sampler2D Texture;
            
            out vec4 Out_Color;
            
            void main()
            {
                Out_Color = Frag_Color * texture(Texture, Frag_UV.st);
            }
        ";
        
        uint vertexShaderId = _gl.CreateShader(ShaderType.VertexShader);
        _gl.ShaderSource(vertexShaderId, vertexShader);
        _gl.CompileShader(vertexShaderId);
        CheckShader(vertexShaderId, "vertex");
        
        uint fragmentShaderId = _gl.CreateShader(ShaderType.FragmentShader);
        _gl.ShaderSource(fragmentShaderId, fragmentShader);
        _gl.CompileShader(fragmentShaderId);
        CheckShader(fragmentShaderId, "fragment");
        
        _shaderProgram = _gl.CreateProgram();
        _gl.AttachShader(_shaderProgram, vertexShaderId);
        _gl.AttachShader(_shaderProgram, fragmentShaderId);
        _gl.LinkProgram(_shaderProgram);
        CheckProgram(_shaderProgram);
        
        _gl.DeleteShader(vertexShaderId);
        _gl.DeleteShader(fragmentShaderId);
        
        _attribLocationTex = _gl.GetUniformLocation(_shaderProgram, "Texture");
        _attribLocationProjMtx = _gl.GetUniformLocation(_shaderProgram, "ProjMtx");
        _attribLocationVtxPos = _gl.GetAttribLocation(_shaderProgram, "Position");
        _attribLocationVtxUV = _gl.GetAttribLocation(_shaderProgram, "UV");
        _attribLocationVtxColor = _gl.GetAttribLocation(_shaderProgram, "Color");
    }
    
    private void CheckShader(uint shader, string name)
    {
        _gl.GetShader(shader, ShaderParameterName.CompileStatus, out int status);
        if (status == 0)
        {
            string log = _gl.GetShaderInfoLog(shader);
            throw new Exception($"Error compiling {name} shader: {log}");
        }
    }
    
    private void CheckProgram(uint program)
    {
        _gl.GetProgram(program, ProgramPropertyARB.LinkStatus, out int status);
        if (status == 0)
        {
            string log = _gl.GetProgramInfoLog(program);
            throw new Exception($"Error linking shader program: {log}");
        }
    }
    
    private unsafe void CreateFontTexture()
    {
        var io = ImGui.GetIO();
        
        // Build texture atlas
        io.Fonts.GetTexDataAsRGBA32(out IntPtr pixels, out int width, out int height, out int bytesPerPixel);
        
        // Create OpenGL texture
        _fontTexture = _gl.GenTexture();
        _gl.BindTexture(TextureTarget.Texture2D, _fontTexture);
        _gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureMinFilter, (int)TextureMinFilter.Linear);
        _gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureMagFilter, (int)TextureMagFilter.Linear);
        _gl.PixelStore(PixelStoreParameter.UnpackRowLength, 0);
        _gl.TexImage2D(TextureTarget.Texture2D, 0, InternalFormat.Rgba, (uint)width, (uint)height, 0, 
            PixelFormat.Rgba, PixelType.UnsignedByte, (void*)pixels);
        
        // Store texture ID
        io.Fonts.SetTexID((IntPtr)_fontTexture);
        io.Fonts.ClearTexData();
    }
    
    private unsafe void RenderImDrawData(ImDrawDataPtr drawData)
    {
        if (drawData.CmdListsCount == 0)
            return;
        
        // Backup GL state
        _gl.GetInteger(GetPName.ActiveTexture, out int lastActiveTexture);
        _gl.ActiveTexture(TextureUnit.Texture0);
        _gl.GetInteger(GetPName.CurrentProgram, out int lastProgram);
        _gl.GetInteger(GetPName.TextureBinding2D, out int lastTexture);
        _gl.GetInteger(GetPName.ArrayBufferBinding, out int lastArrayBuffer);
        _gl.GetInteger(GetPName.VertexArrayBinding, out int lastVertexArray);
        
        Span<int> lastViewport = stackalloc int[4];
        _gl.GetInteger(GetPName.Viewport, lastViewport);
        Span<int> lastScissorBox = stackalloc int[4];
        _gl.GetInteger(GetPName.ScissorBox, lastScissorBox);
        
        _gl.GetInteger(GetPName.BlendSrcRgb, out int lastBlendSrcRgb);
        _gl.GetInteger(GetPName.BlendDstRgb, out int lastBlendDstRgb);
        _gl.GetInteger(GetPName.BlendSrcAlpha, out int lastBlendSrcAlpha);
        _gl.GetInteger(GetPName.BlendDstAlpha, out int lastBlendDstAlpha);
        _gl.GetInteger(GetPName.BlendEquationRgb, out int lastBlendEquationRgb);
        _gl.GetInteger(GetPName.BlendEquationAlpha, out int lastBlendEquationAlpha);
        
        bool lastEnableBlend = _gl.IsEnabled(EnableCap.Blend);
        bool lastEnableCullFace = _gl.IsEnabled(EnableCap.CullFace);
        bool lastEnableDepthTest = _gl.IsEnabled(EnableCap.DepthTest);
        bool lastEnableScissorTest = _gl.IsEnabled(EnableCap.ScissorTest);
        
        // Setup render state
        _gl.Enable(EnableCap.Blend);
        _gl.BlendEquation(BlendEquationModeEXT.FuncAdd);
        _gl.BlendFunc(BlendingFactor.SrcAlpha, BlendingFactor.OneMinusSrcAlpha);
        _gl.Disable(EnableCap.CullFace);
        _gl.Disable(EnableCap.DepthTest);
        _gl.Enable(EnableCap.ScissorTest);
        _gl.PolygonMode(TriangleFace.FrontAndBack, PolygonMode.Fill);
        
        // Setup viewport, orthographic projection matrix
        _gl.Viewport(0, 0, (uint)_window.Size.X, (uint)_window.Size.Y);
        float L = drawData.DisplayPos.X;
        float R = drawData.DisplayPos.X + drawData.DisplaySize.X;
        float T = drawData.DisplayPos.Y;
        float B = drawData.DisplayPos.Y + drawData.DisplaySize.Y;
        
        Span<float> orthoProjection = stackalloc float[]
        {
            2.0f/(R-L),   0.0f,         0.0f,   0.0f,
            0.0f,         2.0f/(T-B),   0.0f,   0.0f,
            0.0f,         0.0f,        -1.0f,   0.0f,
            (R+L)/(L-R),  (T+B)/(B-T),  0.0f,   1.0f,
        };
        
        _gl.UseProgram(_shaderProgram);
        _gl.Uniform1(_attribLocationTex, 0);
        _gl.UniformMatrix4(_attribLocationProjMtx, 1, false, orthoProjection);
        
        _gl.BindVertexArray(_vertexArray);
        
        // Bind vertex/index buffers and setup attributes
        _gl.BindBuffer(BufferTargetARB.ArrayBuffer, _vertexBuffer);
        _gl.BindBuffer(BufferTargetARB.ElementArrayBuffer, _elementBuffer);
        
        _gl.EnableVertexAttribArray((uint)_attribLocationVtxPos);
        _gl.EnableVertexAttribArray((uint)_attribLocationVtxUV);
        _gl.EnableVertexAttribArray((uint)_attribLocationVtxColor);
        
        _gl.VertexAttribPointer((uint)_attribLocationVtxPos, 2, VertexAttribPointerType.Float, false, 
            (uint)Unsafe.SizeOf<ImDrawVert>(), (void*)0);
        _gl.VertexAttribPointer((uint)_attribLocationVtxUV, 2, VertexAttribPointerType.Float, false, 
            (uint)Unsafe.SizeOf<ImDrawVert>(), (void*)8);
        _gl.VertexAttribPointer((uint)_attribLocationVtxColor, 4, VertexAttribPointerType.UnsignedByte, true, 
            (uint)Unsafe.SizeOf<ImDrawVert>(), (void*)16);
        
        // Draw
        Vector2 clipOff = drawData.DisplayPos;
        Vector2 clipScale = drawData.FramebufferScale;
        
        for (int n = 0; n < drawData.CmdListsCount; n++)
        {
            ImDrawListPtr cmdList = drawData.CmdLists[n];
            
            _gl.BufferData(BufferTargetARB.ArrayBuffer, (nuint)(cmdList.VtxBuffer.Size * Unsafe.SizeOf<ImDrawVert>()), 
                (void*)cmdList.VtxBuffer.Data, BufferUsageARB.StreamDraw);
            _gl.BufferData(BufferTargetARB.ElementArrayBuffer, (nuint)(cmdList.IdxBuffer.Size * sizeof(ushort)), 
                (void*)cmdList.IdxBuffer.Data, BufferUsageARB.StreamDraw);
            
            for (int cmdI = 0; cmdI < cmdList.CmdBuffer.Size; cmdI++)
            {
                ImDrawCmdPtr pcmd = cmdList.CmdBuffer[cmdI];
                
                Vector4 clipRect;
                clipRect.X = (pcmd.ClipRect.X - clipOff.X) * clipScale.X;
                clipRect.Y = (pcmd.ClipRect.Y - clipOff.Y) * clipScale.Y;
                clipRect.Z = (pcmd.ClipRect.Z - clipOff.X) * clipScale.X;
                clipRect.W = (pcmd.ClipRect.W - clipOff.Y) * clipScale.Y;
                
                if (clipRect.X < _window.Size.X && clipRect.Y < _window.Size.Y && clipRect.Z >= 0.0f && clipRect.W >= 0.0f)
                {
                    _gl.Scissor((int)clipRect.X, (int)(_window.Size.Y - clipRect.W), 
                        (uint)(clipRect.Z - clipRect.X), (uint)(clipRect.W - clipRect.Y));
                    
                    _gl.BindTexture(TextureTarget.Texture2D, (uint)pcmd.TextureId);
                    _gl.DrawElementsBaseVertex(PrimitiveType.Triangles, pcmd.ElemCount, 
                        DrawElementsType.UnsignedShort, (void*)(pcmd.IdxOffset * sizeof(ushort)), 
                        (int)pcmd.VtxOffset);
                }
            }
        }
        
        // Restore modified GL state
        _gl.UseProgram((uint)lastProgram);
        _gl.BindTexture(TextureTarget.Texture2D, (uint)lastTexture);
        _gl.ActiveTexture((TextureUnit)lastActiveTexture);
        _gl.BindVertexArray((uint)lastVertexArray);
        _gl.BindBuffer(BufferTargetARB.ArrayBuffer, (uint)lastArrayBuffer);
        _gl.BlendEquationSeparate((BlendEquationModeEXT)lastBlendEquationRgb, (BlendEquationModeEXT)lastBlendEquationAlpha);
        _gl.BlendFuncSeparate((BlendingFactor)lastBlendSrcRgb, (BlendingFactor)lastBlendDstRgb, 
            (BlendingFactor)lastBlendSrcAlpha, (BlendingFactor)lastBlendDstAlpha);
        
        if (lastEnableBlend) _gl.Enable(EnableCap.Blend); else _gl.Disable(EnableCap.Blend);
        if (lastEnableCullFace) _gl.Enable(EnableCap.CullFace); else _gl.Disable(EnableCap.CullFace);
        if (lastEnableDepthTest) _gl.Enable(EnableCap.DepthTest); else _gl.Disable(EnableCap.DepthTest);
        if (lastEnableScissorTest) _gl.Enable(EnableCap.ScissorTest); else _gl.Disable(EnableCap.ScissorTest);
        
        _gl.Viewport(lastViewport[0], lastViewport[1], (uint)lastViewport[2], (uint)lastViewport[3]);
        _gl.Scissor(lastScissorBox[0], lastScissorBox[1], (uint)lastScissorBox[2], (uint)lastScissorBox[3]);
    }
    
    public void Dispose()
    {
        if (!_disposed)
        {
            _gl.DeleteBuffer(_vertexBuffer);
            _gl.DeleteBuffer(_elementBuffer);
            _gl.DeleteVertexArray(_vertexArray);
            _gl.DeleteProgram(_shaderProgram);
            _gl.DeleteTexture(_fontTexture);
            
            ImGui.DestroyContext();
            _disposed = true;
        }
        GC.SuppressFinalize(this);
    }
}
