"""
Python wrapper for cunfyooz binary obfuscation engine
Provides a Python interface to the cunfyooz C library functionality
"""
import os
import subprocess
import json
import tempfile
from pathlib import Path
from typing import Dict, Any, Optional, List
import shutil


class CunfyoozWrapper:
    """
    Python wrapper for the cunfyooz binary obfuscation engine.
    
    This class provides a high-level interface to the cunfyooz C library,
    allowing Python applications to perform metamorphic transformations
    on PE binaries.
    """
    
    def __init__(self, cunfyooz_path: Optional[str] = None):
        """
        Initialize the cunfyooz wrapper.
        
        Args:
            cunfyooz_path: Path to the cunfyooz binary. If None, will try to find it automatically.
        """
        self.cunfyooz_path = cunfyooz_path or self._find_cunfyooz_binary()
        
        if not self.cunfyooz_path or not os.path.exists(self.cunfyooz_path):
            raise FileNotFoundError(
                "cunfyooz binary not found. Please build the cunfyooz project first.\n"
                "Expected location: ./bin/cunfyooz or /home/mrnob0dy666/cunfyooz/bin/cunfyooz"
            )
    
    def _find_cunfyooz_binary(self) -> Optional[str]:
        """Find the cunfyooz binary in common locations."""
        possible_paths = [
            "./bin/cunfyooz",
            "/home/mrnob0dy666/cunfyooz/bin/cunfyooz",
            "./cunfyooz",
            "../cunfyooz",
            "/usr/local/bin/cunfyooz"
        ]
        
        for path in possible_paths:
            if os.path.exists(path):
                return path
        
        return None
    
    def transform_binary(
        self, 
        input_path: str, 
        output_path: Optional[str] = None,
        config: Optional[Dict[str, Any]] = None
    ) -> Dict[str, Any]:
        """
        Transform a binary using cunfyooz metamorphic engine.
        
        Args:
            input_path: Path to the input binary to transform
            output_path: Path for the transformed binary. If None, will be auto-generated.
            config: Optional configuration for transformations
            
        Returns:
            Dictionary with transformation results
        """
        if not os.path.exists(input_path):
            raise FileNotFoundError(f"Input binary does not exist: {input_path}")
        
        # Create temporary config file if provided
        temp_config_path = None
        if config:
            temp_config_path = self._create_temp_config(config)
        
        try:
            # Determine output path
            if output_path is None:
                input_dir = os.path.dirname(input_path)
                input_name = os.path.basename(input_path)
                output_path = os.path.join(input_dir, f"cunfyoozed_{input_name}")
            
            # Prepare command
            cmd = [self.cunfyooz_path, input_path]
            
            # If we have a config file, copy it to the current directory as config.json
            # since cunfyooz looks for it there by default
            original_config = None
            if temp_config_path:
                original_config = "config.json"
                if os.path.exists(original_config):
                    # Backup original config
                    shutil.copy2(original_config, f"{original_config}.backup")
                
                shutil.copy2(temp_config_path, original_config)
            
            # Execute transformation
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                timeout=600  # 10 minute timeout for complex transformations
            )
            
            # Restore original config if it existed
            if original_config and os.path.exists(f"{original_config}.backup"):
                shutil.move(f"{original_config}.backup", original_config)
            elif original_config and os.path.exists(original_config):
                os.remove(original_config)  # Remove the temp config we placed
            
            # Check if transformation was successful
            if result.returncode == 0:
                if os.path.exists(output_path):
                    return {
                        "status": "success",
                        "original_path": input_path,
                        "transformed_path": output_path,
                        "output": result.stdout,
                        "error": result.stderr if result.stderr.strip() else None,
                        "transformed_exists": True
                    }
                else:
                    return {
                        "status": "error",
                        "original_path": input_path,
                        "transformed_path": output_path,
                        "output": result.stdout,
                        "error": f"Transformed binary not found at expected location: {output_path}",
                        "transformed_exists": False
                    }
            else:
                return {
                    "status": "error",
                    "original_path": input_path,
                    "transformed_path": output_path,
                    "output": result.stdout,
                    "error": result.stderr,
                    "transformed_exists": False
                }
                
        except subprocess.TimeoutExpired:
            return {
                "status": "error",
                "original_path": input_path,
                "transformed_path": output_path,
                "output": "",
                "error": "Transformation timed out after 10 minutes",
                "transformed_exists": False
            }
        except Exception as e:
            return {
                "status": "error",
                "original_path": input_path,
                "transformed_path": output_path,
                "output": "",
                "error": str(e),
                "transformed_exists": False
            }
        finally:
            # Clean up temporary config file
            if temp_config_path and os.path.exists(temp_config_path):
                os.remove(temp_config_path)
    
    def _create_temp_config(self, config: Dict[str, Any]) -> str:
        """Create a temporary config file from the provided configuration."""
        temp_file = tempfile.NamedTemporaryFile(mode='w', suffix='.json', delete=False)
        json.dump(config, temp_file, indent=2)
        temp_file.close()
        return temp_file.name
    
    def analyze_binary(self, binary_path: str) -> Dict[str, Any]:
        """
        Analyze a binary file to extract information about its structure.
        
        Args:
            binary_path: Path to the binary to analyze
            
        Returns:
            Dictionary with analysis results
        """
        if not os.path.exists(binary_path):
            raise FileNotFoundError(f"Binary does not exist: {binary_path}")
        
        # Get file stats
        stat_info = os.stat(binary_path)
        size = stat_info.st_size
        
        # Determine file type
        with open(binary_path, 'rb') as f:
            magic = f.read(4)

        file_type = "Unknown"
        if magic.startswith(b'\x7fELF'):  # ELF file (0x7f 'E' 'L' 'F')
            file_type = "ELF (Executable and Linkable Format)"
        elif magic.startswith(b'MZ'):  # PE file
            file_type = "PE (Portable Executable)"
        elif len(magic) >= 2 and magic[:2] in [b'\x4d\x5a', b'\x5a\x4d']:  # MZ or ZM
            file_type = "PE (Portable Executable)"
        
        # Calculate entropy
        entropy = self._calculate_entropy(binary_path)
        
        return {
            "path": binary_path,
            "size": size,
            "type": file_type,
            "entropy": entropy,
            "analysis_completed": True
        }
    
    def _calculate_entropy(self, file_path: str) -> float:
        """Calculate the entropy of a file."""
        with open(file_path, 'rb') as f:
            data = f.read()
        
        if len(data) == 0:
            return 0.0
        
        from collections import Counter
        import math
        
        byte_counts = Counter(data)
        file_size = len(data)
        
        entropy = 0.0
        for count in byte_counts.values():
            probability = count / file_size
            entropy -= probability * math.log2(probability)
        
        return round(entropy, 4)
    
    def compare_binaries(self, original_path: str, transformed_path: str) -> Dict[str, Any]:
        """
        Compare two binary files to assess transformation impact.
        
        Args:
            original_path: Path to the original binary
            transformed_path: Path to the transformed binary
            
        Returns:
            Dictionary with comparison results
        """
        if not os.path.exists(original_path):
            raise FileNotFoundError(f"Original binary does not exist: {original_path}")
        if not os.path.exists(transformed_path):
            raise FileNotFoundError(f"Transformed binary does not exist: {transformed_path}")
        
        # Analyze both binaries
        orig_analysis = self.analyze_binary(original_path)
        trans_analysis = self.analyze_binary(transformed_path)
        
        # Calculate size difference
        size_diff = trans_analysis['size'] - orig_analysis['size']
        size_change_percent = (size_diff / orig_analysis['size']) * 100 if orig_analysis['size'] > 0 else 0
        
        # Calculate hash to check if content is identical
        orig_hash = self._calculate_file_hash(original_path)
        trans_hash = self._calculate_file_hash(transformed_path)
        content_identical = orig_hash == trans_hash
        
        return {
            "original": orig_analysis,
            "transformed": trans_analysis,
            "size_difference": size_diff,
            "size_change_percent": round(size_change_percent, 2),
            "content_identical": content_identical,
            "original_hash": orig_hash,
            "transformed_hash": trans_hash,
            "comparison_completed": True
        }
    
    def _calculate_file_hash(self, file_path: str) -> str:
        """Calculate SHA256 hash of a file."""
        import hashlib
        
        hash_sha256 = hashlib.sha256()
        with open(file_path, "rb") as f:
            for chunk in iter(lambda: f.read(4096), b""):
                hash_sha256.update(chunk)
        return hash_sha256.hexdigest()


class CunfyoozPipeline:
    """
    A pipeline for performing multiple binary transformation operations.
    """
    
    def __init__(self, cunfyooz_path: Optional[str] = None):
        self.wrapper = CunfyoozWrapper(cunfyooz_path)
    
    def run_complete_transformation(
        self, 
        input_path: str, 
        config: Optional[Dict[str, Any]] = None
    ) -> Dict[str, Any]:
        """
        Run a complete transformation pipeline: analyze, transform, and compare.
        
        Args:
            input_path: Path to the input binary
            config: Optional transformation configuration
            
        Returns:
            Dictionary with complete pipeline results
        """
        # 1. Analyze original binary
        print(f"Analyzing original binary: {input_path}")
        original_analysis = self.wrapper.analyze_binary(input_path)
        
        # 2. Transform binary
        print("Transforming binary with cunfyooz...")
        transform_result = self.wrapper.transform_binary(input_path, config=config)
        
        if transform_result["status"] == "error":
            return {
                "status": "error",
                "original_analysis": original_analysis,
                "transform_result": transform_result
            }
        
        # 3. Analyze transformed binary
        print(f"Analyzing transformed binary: {transform_result['transformed_path']}")
        transformed_analysis = self.wrapper.analyze_binary(transform_result['transformed_path'])
        
        # 4. Compare binaries
        print("Comparing original and transformed binaries...")
        comparison = self.wrapper.compare_binaries(
            input_path, 
            transform_result['transformed_path']
        )
        
        return {
            "status": "success",
            "original_analysis": original_analysis,
            "transform_result": transform_result,
            "transformed_analysis": transformed_analysis,
            "comparison": comparison,
            "pipeline_completed": True
        }


# Example usage function
def example_usage():
    """Example of how to use the CunfyoozWrapper."""
    try:
        # Initialize wrapper
        cunfyooz = CunfyoozWrapper()
        
        # Example transformation config
        config = {
            "transformations": {
                "nop_insertion": {
                    "enabled": True,
                    "probability": 5
                },
                "instruction_substitution": {
                    "enabled": True,
                    "probability": 10
                },
                "register_shuffling": {
                    "enabled": True,
                    "probability": 8
                },
                "enhanced_nop_insertion": {
                    "enabled": True,
                    "probability": 3
                },
                "control_flow_obfuscation": {
                    "enabled": True,
                    "probability": 5
                },
                "stack_frame_obfuscation": {
                    "enabled": True,
                    "probability": 2
                },
                "instruction_reordering": {
                    "enabled": True,
                    "probability": 5
                },
                "anti_analysis_techniques": {
                    "enabled": True,
                    "probability": 15
                },
                "virtualization_engine": {
                    "enabled": False,
                    "probability": 10
                }
            },
            "output": {
                "verbose": True,
                "log_transformations": True
            },
            "security": {
                "validate_functionality": False,  # Changed from True to False for security
                "preserve_original_behavior": True
            }
        }
        
        # Perform transformation (this would require an actual binary file)
        # result = cunfyooz.transform_binary("path/to/binary.exe", config=config)
        # print(result)
        
        print("CunfyoozWrapper initialized successfully")
        
    except FileNotFoundError as e:
        print(f"Error: {e}")
    except Exception as e:
        print(f"Unexpected error: {e}")


if __name__ == "__main__":
    example_usage()