using AvorionLike.Core.Scripting;

namespace AvorionLike.Core.DevTools;

/// <summary>
/// Debug Console - Runtime command console for development and debugging
/// Activated with the ` (backtick) key
/// </summary>
public class DebugConsole
{
    private bool isVisible = false;
    private List<string> commandHistory = new();
    private List<string> outputHistory = new();
    private int historyIndex = -1;
    private string currentInput = "";
    private Dictionary<string, ConsoleCommand> commands = new();
    private ScriptingEngine? scriptingEngine;

    public bool IsVisible
    {
        get => isVisible;
        set => isVisible = value;
    }

    public IReadOnlyList<string> OutputHistory => outputHistory.AsReadOnly();

    public DebugConsole(ScriptingEngine? scripting = null)
    {
        scriptingEngine = scripting;
        RegisterDefaultCommands();
    }

    /// <summary>
    /// Toggle console visibility
    /// </summary>
    public void Toggle()
    {
        isVisible = !isVisible;
        if (isVisible)
        {
            WriteLine("Debug Console (Type 'help' for commands)");
        }
    }

    /// <summary>
    /// Register a console command
    /// </summary>
    public void RegisterCommand(string name, string description, Action<string[]> action)
    {
        commands[name.ToLower()] = new ConsoleCommand
        {
            Name = name,
            Description = description,
            Action = action
        };
    }

    /// <summary>
    /// Execute a console command
    /// </summary>
    public void ExecuteCommand(string input)
    {
        if (string.IsNullOrWhiteSpace(input))
            return;

        // Add to history
        commandHistory.Add(input);
        historyIndex = commandHistory.Count;
        
        // Echo command
        WriteLine($"> {input}");

        // Parse command
        string[] parts = input.Split(' ', StringSplitOptions.RemoveEmptyEntries);
        if (parts.Length == 0)
            return;

        string commandName = parts[0].ToLower();
        string[] args = parts.Skip(1).ToArray();

        // Execute command
        if (commands.ContainsKey(commandName))
        {
            try
            {
                commands[commandName].Action(args);
            }
            catch (Exception ex)
            {
                WriteLine($"Error executing command: {ex.Message}");
            }
        }
        else
        {
            WriteLine($"Unknown command: {commandName}. Type 'help' for available commands.");
        }

        currentInput = "";
    }

    /// <summary>
    /// Write a line to the console output
    /// </summary>
    public void WriteLine(string message)
    {
        outputHistory.Add(message);
        Console.WriteLine($"[Debug Console] {message}");
    }

    /// <summary>
    /// Clear the console output
    /// </summary>
    public void Clear()
    {
        outputHistory.Clear();
        WriteLine("Console cleared.");
    }

    /// <summary>
    /// Navigate command history
    /// </summary>
    public void NavigateHistory(int direction)
    {
        if (commandHistory.Count == 0)
            return;

        historyIndex = Math.Clamp(historyIndex + direction, 0, commandHistory.Count);
        
        if (historyIndex < commandHistory.Count)
            currentInput = commandHistory[historyIndex];
        else
            currentInput = "";
    }

    /// <summary>
    /// Get current input
    /// </summary>
    public string GetCurrentInput() => currentInput;

    /// <summary>
    /// Set current input
    /// </summary>
    public void SetCurrentInput(string input) => currentInput = input;

    /// <summary>
    /// Register default console commands
    /// </summary>
    private void RegisterDefaultCommands()
    {
        RegisterCommand("help", "Show all available commands", args =>
        {
            WriteLine("Available Commands:");
            foreach (var cmd in commands.Values.OrderBy(c => c.Name))
            {
                WriteLine($"  {cmd.Name} - {cmd.Description}");
            }
        });

        RegisterCommand("clear", "Clear console output", args => Clear());

        RegisterCommand("exit", "Close the console", args => isVisible = false);

        RegisterCommand("echo", "Echo text to console", args =>
        {
            WriteLine(string.Join(" ", args));
        });

        RegisterCommand("history", "Show command history", args =>
        {
            WriteLine("Command History:");
            for (int i = 0; i < commandHistory.Count; i++)
            {
                WriteLine($"  {i + 1}. {commandHistory[i]}");
            }
        });

        RegisterCommand("lua", "Execute Lua script", args =>
        {
            if (scriptingEngine == null)
            {
                WriteLine("Scripting engine not available");
                return;
            }

            if (args.Length == 0)
            {
                WriteLine("Usage: lua <script>");
                return;
            }

            string script = string.Join(" ", args);
            try
            {
                scriptingEngine.ExecuteScript(script);
                WriteLine("Script executed successfully");
            }
            catch (Exception ex)
            {
                WriteLine($"Script error: {ex.Message}");
            }
        });

        RegisterCommand("gc", "Force garbage collection", args =>
        {
            long before = GC.GetTotalMemory(false);
            GC.Collect();
            GC.WaitForPendingFinalizers();
            GC.Collect();
            long after = GC.GetTotalMemory(false);
            WriteLine($"Garbage collection completed. Freed: {(before - after) / 1024.0:F2} KB");
        });

        RegisterCommand("mem", "Show memory usage", args =>
        {
            var process = System.Diagnostics.Process.GetCurrentProcess();
            WriteLine($"Working Set: {process.WorkingSet64 / (1024.0 * 1024.0):F2} MB");
            WriteLine($"Managed Memory: {GC.GetTotalMemory(false) / (1024.0 * 1024.0):F2} MB");
        });
    }

    private class ConsoleCommand
    {
        public string Name { get; set; } = "";
        public string Description { get; set; } = "";
        public Action<string[]> Action { get; set; } = _ => { };
    }
}
