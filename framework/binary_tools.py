"""
Binary Analysis and Transformation Tools for Claude API Agents
Provides specialized tools for binary operations.
"""
from typing import Dict, List, Any
import os
import subprocess
import struct
import hashlib
from pathlib import Path


class BinaryToolDefinitions:
    """
    Specialized tool definitions for binary analysis and transformation.
    """

    @staticmethod
    def binary_info() -> Dict[str, Any]:
        """Tool for getting basic information about a binary file"""
        return {
            "name": "binary_info",
            "description": "Get basic information about a binary file (size, type, architecture)",
            "input_schema": {
                "type": "object",
                "properties": {
                    "path": {
                        "type": "string",
                        "description": "Path to the binary file"
                    }
                },
                "required": ["path"]
            }
        }

    @staticmethod
    def binary_compare() -> Dict[str, Any]:
        """Tool for comparing two binary files"""
        return {
            "name": "binary_compare",
            "description": "Compare two binary files and report differences",
            "input_schema": {
                "type": "object",
                "properties": {
                    "original_path": {
                        "type": "string",
                        "description": "Path to the original binary file"
                    },
                    "transformed_path": {
                        "type": "string",
                        "description": "Path to the transformed binary file"
                    }
                },
                "required": ["original_path", "transformed_path"]
            }
        }

    @staticmethod
    def binary_entropy() -> Dict[str, Any]:
        """Tool for calculating entropy of a binary file"""
        return {
            "name": "binary_entropy",
            "description": "Calculate entropy of a binary file to assess randomness",
            "input_schema": {
                "type": "object",
                "properties": {
                    "path": {
                        "type": "string",
                        "description": "Path to the binary file"
                    }
                },
                "required": ["path"]
            }
        }

    @staticmethod
    def pe_analysis() -> Dict[str, Any]:
        """Tool for PE-specific analysis"""
        return {
            "name": "pe_analysis",
            "description": "Analyze PE (Portable Executable) file structure and properties",
            "input_schema": {
                "type": "object",
                "properties": {
                    "path": {
                        "type": "string",
                        "description": "Path to the PE file"
                    }
                },
                "required": ["path"]
            }
        }

    @staticmethod
    def run_cunfyooz() -> Dict[str, Any]:
        """Tool for running cunfyooz transformation on a binary"""
        return {
            "name": "run_cunfyooz",
            "description": "Run cunfyooz metamorphic transformation on a binary file",
            "input_schema": {
                "type": "object",
                "properties": {
                    "binary_path": {
                        "type": "string",
                        "description": "Path to the binary file to transform"
                    },
                    "config_path": {
                        "type": "string",
                        "description": "Optional path to cunfyooz configuration file"
                    }
                },
                "required": ["binary_path"]
            }
        }

    @staticmethod
    def get_all_binary_tools() -> List[Dict[str, Any]]:
        """Get all binary analysis tools as a list"""
        return [
            BinaryToolDefinitions.binary_info(),
            BinaryToolDefinitions.binary_compare(),
            BinaryToolDefinitions.binary_entropy(),
            BinaryToolDefinitions.pe_analysis(),
            BinaryToolDefinitions.run_cunfyooz(),
        ]


class BinaryToolExecutor:
    """
    Executes binary analysis and transformation tools.
    """

    def __init__(self):
        self.handlers = {
            "binary_info": self._handle_binary_info,
            "binary_compare": self._handle_binary_compare,
            "binary_entropy": self._handle_binary_entropy,
            "pe_analysis": self._handle_pe_analysis,
            "run_cunfyooz": self._handle_run_cunfyooz,
        }

    def execute_tool(self, tool_name: str, tool_input: Dict[str, Any]) -> Any:
        """
        Execute a binary tool call.

        Args:
            tool_name: Name of the tool to execute
            tool_input: Input parameters for the tool

        Returns:
            Tool execution result
        """
        if tool_name not in self.handlers:
            raise ValueError(f"No handler registered for tool: {tool_name}")

        handler = self.handlers[tool_name]
        return handler(tool_input)

    def _handle_binary_info(self, tool_input: Dict[str, Any]) -> str:
        """Handle binary info tool"""
        path = tool_input["path"]
        
        if not os.path.exists(path):
            return f"Error: File does not exist: {path}"
        
        try:
            stat_info = os.stat(path)
            size = stat_info.st_size
            
            # Determine file type by reading magic bytes
            with open(path, 'rb') as f:
                magic = f.read(4)
            
            file_type = "Unknown"
            if magic.startswith(b'MZ'):  # PE file
                file_type = "PE (Portable Executable)"
            elif magic.startswith(b'ELF'):  # ELF file
                file_type = "ELF (Executable and Linkable Format)"
            elif magic.startswith(b'\x7fELF'):  # ELF file (with 0x7f)
                file_type = "ELF (Executable and Linkable Format)"
            elif len(magic) >= 2 and magic[:2] in [b'\x4d\x5a', b'\x5a\x4d']:  # MZ or ZM
                file_type = "PE (Portable Executable)"
            
            return f"Binary Info:\n- Size: {size} bytes\n- Type: {file_type}\n- Path: {path}"
        except Exception as e:
            return f"Error reading file: {str(e)}"

    def _handle_binary_compare(self, tool_input: Dict[str, Any]) -> str:
        """Handle binary compare tool"""
        original_path = tool_input["original_path"]
        transformed_path = tool_input["transformed_path"]
        
        if not os.path.exists(original_path):
            return f"Error: Original file does not exist: {original_path}"
        if not os.path.exists(transformed_path):
            return f"Error: Transformed file does not exist: {transformed_path}"
        
        try:
            # Get file sizes
            orig_size = os.path.getsize(original_path)
            trans_size = os.path.getsize(transformed_path)
            
            # Calculate hashes
            def get_hash(filepath):
                hash_sha256 = hashlib.sha256()
                with open(filepath, "rb") as f:
                    for chunk in iter(lambda: f.read(4096), b""):
                        hash_sha256.update(chunk)
                return hash_sha256.hexdigest()
            
            orig_hash = get_hash(original_path)
            trans_hash = get_hash(transformed_path)
            
            same_content = orig_hash == trans_hash
            
            return f"Binary Comparison:\n- Original size: {orig_size} bytes\n- Transformed size: {trans_size} bytes\n- Size difference: {trans_size - orig_size} bytes\n- Content identical: {'Yes' if same_content else 'No'}\n- Original hash: {orig_hash}\n- Transformed hash: {trans_hash}"
        except Exception as e:
            return f"Error comparing files: {str(e)}"

    def _handle_binary_entropy(self, tool_input: Dict[str, Any]) -> str:
        """Handle binary entropy calculation"""
        path = tool_input["path"]
        
        if not os.path.exists(path):
            return f"Error: File does not exist: {path}"
        
        try:
            with open(path, 'rb') as f:
                data = f.read()
            
            if len(data) == 0:
                return "Entropy: 0.0 (empty file)"
            
            # Calculate entropy
            from collections import Counter
            import math
            
            byte_counts = Counter(data)
            file_size = len(data)
            
            entropy = 0.0
            for count in byte_counts.values():
                probability = count / file_size
                entropy -= probability * math.log2(probability)
            
            return f"Entropy: {entropy:.4f} bits per byte\n- File: {path}\n- Size: {len(data)} bytes\n- Higher entropy indicates more randomness"
        except Exception as e:
            return f"Error calculating entropy: {str(e)}"

    def _handle_pe_analysis(self, tool_input: Dict[str, Any]) -> str:
        """Handle PE analysis"""
        path = tool_input["path"]
        
        if not os.path.exists(path):
            return f"Error: File does not exist: {path}"
        
        try:
            with open(path, 'rb') as f:
                # Read DOS header
                dos_magic = f.read(2)
                if dos_magic != b'MZ':
                    return "Error: Not a valid PE file (missing MZ header)"
                
                # Skip to PE header offset
                f.seek(0x3C)
                pe_offset_bytes = f.read(4)
                pe_offset = struct.unpack('<I', pe_offset_bytes)[0]
                
                # Read PE header
                f.seek(pe_offset)
                pe_magic = f.read(4)
                if pe_magic != b'PE\x00\x00':
                    return "Error: Not a valid PE file (missing PE header)"
                
                # Read COFF header (after PE signature)
                machine_bytes = f.read(2)
                machine = struct.unpack('<H', machine_bytes)[0]
                
                # Determine architecture
                arch_map = {
                    0x14c: 'x86 (32-bit)',
                    0x8664: 'x64 (64-bit)',
                    0x1c0: 'ARM',
                    0xaa64: 'ARM64'
                }
                architecture = arch_map.get(machine, f'Unknown (0x{machine:x})')
                
                # Read optional header
                characteristics_bytes = f.read(20)  # Skip various fields
                magic = struct.unpack('<H', characteristics_bytes[0:2])[0]
                
                # Determine PE format
                pe_format = "PE32" if magic == 0x10b else "PE32+" if magic == 0x20b else f"Unknown (0x{magic:x})"
                
                return f"PE Analysis:\n- Architecture: {architecture}\n- Format: {pe_format}\n- Path: {path}\n- Valid PE file with standard headers"
        except Exception as e:
            return f"Error analyzing PE file: {str(e)}"

    def _handle_run_cunfyooz(self, tool_input: Dict[str, Any]) -> str:
        """Handle running cunfyooz transformation"""
        binary_path = tool_input["binary_path"]
        config_path = tool_input.get("config_path")
        
        if not os.path.exists(binary_path):
            return f"Error: Binary file does not exist: {binary_path}"
        
        if config_path and not os.path.exists(config_path):
            return f"Error: Config file does not exist: {config_path}"
        
        try:
            # Check if cunfyooz binary exists
            cunfyooz_path = "./bin/cunfyooz"
            if not os.path.exists(cunfyooz_path):
                # Try to find it in the project root
                cunfyooz_path = "/home/mrnob0dy666/cunfyooz/bin/cunfyooz"
                if not os.path.exists(cunfyooz_path):
                    return "Error: cunfyooz binary not found. Please build the cunfyooz project first."
            
            # Prepare command
            cmd = [cunfyooz_path, binary_path]
            
            # Execute cunfyooz transformation
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                timeout=300  # 5 minute timeout
            )
            
            if result.returncode == 0:
                # Find the transformed binary (should be named cunfyoozed_*)
                original_dir = os.path.dirname(binary_path)
                original_name = os.path.basename(binary_path)
                transformed_name = f"cunfyoozed_{original_name}"
                transformed_path = os.path.join(original_dir, transformed_name)
                
                if os.path.exists(transformed_path):
                    return f"Cunfyooz transformation completed successfully:\n- Original: {binary_path}\n- Transformed: {transformed_path}\n- Output: {result.stdout}"
                else:
                    return f"Cunfyooz transformation completed but transformed binary not found at expected location: {transformed_path}\n- Output: {result.stdout}\n- Error: {result.stderr}"
            else:
                return f"Cunfyooz transformation failed with return code {result.returncode}:\n- Error: {result.stderr}\n- Output: {result.stdout}"
                
        except subprocess.TimeoutExpired:
            return "Error: cunfyooz transformation timed out after 5 minutes"
        except Exception as e:
            return f"Error executing cunfyooz: {str(e)}"