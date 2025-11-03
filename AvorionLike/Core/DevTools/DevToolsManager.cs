using AvorionLike.Core.Scripting;

namespace AvorionLike.Core.DevTools;

/// <summary>
/// Development Tools Manager - Manages all development and debugging tools
/// </summary>
public class DevToolsManager
{
    public DebugRenderer DebugRenderer { get; private set; }
    public PerformanceProfiler PerformanceProfiler { get; private set; }
    public MemoryTracker MemoryTracker { get; private set; }
    public OpenGLDebugger OpenGLDebugger { get; private set; }
    public DebugConsole DebugConsole { get; private set; }
    public ScriptCompiler ScriptCompiler { get; private set; }

    private bool isInitialized = false;
    public bool IsEnabled { get; set; } = true;

    public DevToolsManager(ScriptingEngine scriptingEngine)
    {
        DebugRenderer = new DebugRenderer();
        PerformanceProfiler = new PerformanceProfiler();
        MemoryTracker = new MemoryTracker();
        OpenGLDebugger = new OpenGLDebugger();
        DebugConsole = new DebugConsole(scriptingEngine);
        ScriptCompiler = new ScriptCompiler(scriptingEngine);

        RegisterConsoleCommands();
        isInitialized = true;
    }

    /// <summary>
    /// Update all development tools
    /// </summary>
    public void Update(float deltaTime)
    {
        if (!IsEnabled || !isInitialized)
            return;

        PerformanceProfiler.EndFrame();
        PerformanceProfiler.BeginFrame();

        DebugRenderer.Update(deltaTime);
        MemoryTracker.Update();
    }

    /// <summary>
    /// Handle console input (check for backtick key to toggle console)
    /// </summary>
    public void HandleInput(ConsoleKeyInfo keyInfo)
    {
        if (keyInfo.KeyChar == '`' || keyInfo.KeyChar == '~')
        {
            DebugConsole.Toggle();
            return;
        }

        if (DebugConsole.IsVisible)
        {
            HandleConsoleInput(keyInfo);
        }
    }

    /// <summary>
    /// Handle console-specific input
    /// </summary>
    private void HandleConsoleInput(ConsoleKeyInfo keyInfo)
    {
        switch (keyInfo.Key)
        {
            case ConsoleKey.Enter:
                DebugConsole.ExecuteCommand(DebugConsole.GetCurrentInput());
                break;
            case ConsoleKey.UpArrow:
                DebugConsole.NavigateHistory(-1);
                break;
            case ConsoleKey.DownArrow:
                DebugConsole.NavigateHistory(1);
                break;
            case ConsoleKey.Backspace:
                var input = DebugConsole.GetCurrentInput();
                if (input.Length > 0)
                    DebugConsole.SetCurrentInput(input.Substring(0, input.Length - 1));
                break;
            default:
                if (!char.IsControl(keyInfo.KeyChar))
                {
                    DebugConsole.SetCurrentInput(DebugConsole.GetCurrentInput() + keyInfo.KeyChar);
                }
                break;
        }
    }

    /// <summary>
    /// Register console commands for dev tools
    /// </summary>
    private void RegisterConsoleCommands()
    {
        DebugConsole.RegisterCommand("fps", "Show FPS and performance info", args =>
        {
            DebugConsole.WriteLine($"FPS: {PerformanceProfiler.CurrentFPS:F2}");
            DebugConsole.WriteLine($"Avg Frame Time: {PerformanceProfiler.AverageFrameTime:F2}ms");
            DebugConsole.WriteLine($"Frame Count: {PerformanceProfiler.FrameCount}");
        });

        DebugConsole.RegisterCommand("profile", "Show performance profile report", args =>
        {
            DebugConsole.WriteLine(PerformanceProfiler.GenerateReport());
        });

        DebugConsole.RegisterCommand("memory", "Show memory usage report", args =>
        {
            DebugConsole.WriteLine(MemoryTracker.GenerateReport());
        });

        DebugConsole.RegisterCommand("glerrors", "Show OpenGL error report", args =>
        {
            DebugConsole.WriteLine(OpenGLDebugger.GenerateReport());
        });

        DebugConsole.RegisterCommand("scripts", "Show loaded scripts", args =>
        {
            DebugConsole.WriteLine(ScriptCompiler.GenerateReport());
        });

        DebugConsole.RegisterCommand("debug", "Toggle debug rendering", args =>
        {
            DebugRenderer.IsEnabled = !DebugRenderer.IsEnabled;
            DebugConsole.WriteLine($"Debug rendering: {(DebugRenderer.IsEnabled ? "Enabled" : "Disabled")}");
        });

        DebugConsole.RegisterCommand("devtools", "Show development tools status", args =>
        {
            DebugConsole.WriteLine("=== Development Tools Status ===");
            DebugConsole.WriteLine($"Debug Renderer: {(DebugRenderer.IsEnabled ? "Enabled" : "Disabled")}");
            DebugConsole.WriteLine($"  Lines: {DebugRenderer.GetLineCount()}");
            DebugConsole.WriteLine($"  Boxes: {DebugRenderer.GetBoxCount()}");
            DebugConsole.WriteLine($"Performance Profiler: Active");
            DebugConsole.WriteLine($"  FPS: {PerformanceProfiler.CurrentFPS:F2}");
            DebugConsole.WriteLine($"  Frames: {PerformanceProfiler.FrameCount}");
            DebugConsole.WriteLine($"Memory Tracker: Active");
            DebugConsole.WriteLine($"  Usage: {MemoryTracker.MemoryUsageMB:F2} MB");
            DebugConsole.WriteLine($"OpenGL Debugger: {(OpenGLDebugger.IsEnabled ? "Enabled" : "Disabled")}");
            DebugConsole.WriteLine($"  Errors: {OpenGLDebugger.ErrorCount}");
            DebugConsole.WriteLine($"Script Compiler: Active");
            DebugConsole.WriteLine($"  Loaded Scripts: {ScriptCompiler.LoadedScriptCount}");
        });

        DebugConsole.RegisterCommand("reload", "Reload a script", args =>
        {
            if (args.Length == 0)
            {
                DebugConsole.WriteLine("Usage: reload <script_name>");
                return;
            }

            if (ScriptCompiler.ReloadScript(args[0]))
                DebugConsole.WriteLine($"Successfully reloaded: {args[0]}");
            else
                DebugConsole.WriteLine($"Failed to reload: {args[0]}");
        });

        DebugConsole.RegisterCommand("compile", "Compile a script file", args =>
        {
            if (args.Length == 0)
            {
                DebugConsole.WriteLine("Usage: compile <file_path>");
                return;
            }

            if (ScriptCompiler.CompileFile(args[0]))
                DebugConsole.WriteLine($"Successfully compiled: {args[0]}");
            else
                DebugConsole.WriteLine($"Failed to compile: {args[0]}");
        });
    }

    /// <summary>
    /// Generate a comprehensive development tools report
    /// </summary>
    public string GenerateFullReport()
    {
        var report = new System.Text.StringBuilder();
        report.AppendLine("╔════════════════════════════════════════════════════════════╗");
        report.AppendLine("║        DEVELOPMENT TOOLS - COMPREHENSIVE REPORT          ║");
        report.AppendLine("╚════════════════════════════════════════════════════════════╝");
        report.AppendLine();
        
        report.AppendLine(PerformanceProfiler.GenerateReport());
        report.AppendLine();
        report.AppendLine(MemoryTracker.GenerateReport());
        report.AppendLine();
        report.AppendLine(OpenGLDebugger.GenerateReport());
        report.AppendLine();
        report.AppendLine(ScriptCompiler.GenerateReport());
        
        return report.ToString();
    }

    /// <summary>
    /// Render development tools UI (for future GUI implementation)
    /// </summary>
    public void RenderUI()
    {
        if (!IsEnabled || !isInitialized)
            return;

        // In a full implementation with a GUI framework, this would render:
        // - Performance graphs
        // - Memory usage graphs
        // - Debug visualization overlay
        // - Console window
        // - Script editor
    }
}
