using Silk.NET.OpenGL;
using System.Numerics;

namespace AvorionLike.Core.Graphics;

/// <summary>
/// OpenGL Shader program wrapper
/// Handles shader compilation, linking, and uniform setting
/// </summary>
public class Shader : IDisposable
{
    private readonly GL _gl;
    private uint _handle;
    private bool _disposed = false;

    public Shader(GL gl, string vertexSource, string fragmentSource)
    {
        _gl = gl;
        
        uint vertex = CompileShader(ShaderType.VertexShader, vertexSource);
        uint fragment = CompileShader(ShaderType.FragmentShader, fragmentSource);
        
        _handle = _gl.CreateProgram();
        _gl.AttachShader(_handle, vertex);
        _gl.AttachShader(_handle, fragment);
        _gl.LinkProgram(_handle);
        
        _gl.GetProgram(_handle, GLEnum.LinkStatus, out var status);
        if (status == 0)
        {
            throw new Exception($"Shader linking failed: {_gl.GetProgramInfoLog(_handle)}");
        }
        
        _gl.DetachShader(_handle, vertex);
        _gl.DetachShader(_handle, fragment);
        _gl.DeleteShader(vertex);
        _gl.DeleteShader(fragment);
    }

    public void Use()
    {
        _gl.UseProgram(_handle);
    }

    public unsafe void SetMatrix4(string name, Matrix4x4 matrix)
    {
        int location = _gl.GetUniformLocation(_handle, name);
        if (location == -1)
            return;
        
        _gl.UniformMatrix4(location, 1, false, (float*)&matrix);
    }

    public void SetVector3(string name, Vector3 value)
    {
        int location = _gl.GetUniformLocation(_handle, name);
        if (location == -1)
            return;
        
        _gl.Uniform3(location, value.X, value.Y, value.Z);
    }

    public void SetFloat(string name, float value)
    {
        int location = _gl.GetUniformLocation(_handle, name);
        if (location == -1)
            return;
        
        _gl.Uniform1(location, value);
    }

    private uint CompileShader(ShaderType type, string source)
    {
        uint shader = _gl.CreateShader(type);
        _gl.ShaderSource(shader, source);
        _gl.CompileShader(shader);
        
        _gl.GetShader(shader, ShaderParameterName.CompileStatus, out var status);
        if (status == 0)
        {
            throw new Exception($"Shader compilation failed ({type}): {_gl.GetShaderInfoLog(shader)}");
        }
        
        return shader;
    }

    public void Dispose()
    {
        if (!_disposed)
        {
            _gl.DeleteProgram(_handle);
            _disposed = true;
        }
        GC.SuppressFinalize(this);
    }
}
