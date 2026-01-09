"""
Cunfyooz Agent Implementation
Specialized agent for binary obfuscation and metamorphic transformations.
"""
from typing import Dict, List, Any, Optional
import sys
import subprocess
import os
from pathlib import Path

# Add parent directory to path for imports
sys.path.insert(0, str(Path(__file__).parent.parent))

from framework import BaseAgent, ToolDefinitions
from framework.tools import ToolExecutor


class CunfyoozAgent(BaseAgent):
    """
    Specialized agent for binary obfuscation using the cunfyooz metamorphic engine.

    This agent can:
    - Apply metamorphic transformations to PE binaries
    - Analyze binary structure and properties
    - Validate transformed binaries
    - Report on obfuscation effectiveness
    """

    def __init__(self, config: Dict[str, Any]):
        super().__init__(
            agent_id="cunfyooz_agent",
            name="Cunfyooz Binary Obfuscation Agent",
            description="Performs metamorphic transformations on PE binaries to create functionally equivalent but structurally different variants",
            capabilities=[
                "PE binary parsing",
                "Metamorphic code generation",
                "Binary obfuscation",
                "NOP insertion",
                "Instruction substitution",
                "Register shuffling",
                "Control flow obfuscation",
                "Binary validation"
            ],
            config=config
        )

    def get_tools(self) -> List[Dict[str, Any]]:
        """Define tools this agent can use"""
        return [
            ToolDefinitions.file_read(),
            ToolDefinitions.file_write(),
            ToolDefinitions.run_command(),
            ToolDefinitions.file_search(),
            ToolDefinitions.web_fetch(),
        ]

    def run(self, task: str, context: Optional[Dict[str, Any]] = None) -> Dict[str, Any]:
        """
        Execute cunfyooz transformation task.

        Args:
            task: Task description (e.g., "obfuscate binary at path/to/file.exe")
            context: Optional context from previous agents

        Returns:
            Transformation results and generated artifacts
        """
        print(f"[CunfyoozAgent] Starting binary transformation: {task}")

        # Extract binary path from task
        binary_path = self._extract_binary_path(task)
        if not binary_path:
            return {
                "status": "error",
                "error": "No binary path specified in task",
                "findings": None,
                "artifacts": []
            }

        # Verify binary exists
        if not os.path.exists(binary_path):
            return {
                "status": "error",
                "error": f"Binary file does not exist: {binary_path}",
                "findings": None,
                "artifacts": []
            }

        # Build system prompt
        system_prompt = f"""You are a binary analysis and obfuscation expert.
Your capabilities: {', '.join(self.capabilities)}

Task: {task}

Binary to analyze: {binary_path}

Analyze the binary and provide recommendations for obfuscation techniques."""
        
        if context:
            system_prompt += f"\nContext from previous agents:\n{context}"

        # Build messages for Claude
        messages = [
            {
                "role": "user",
                "content": f"""Analyze the following binary obfuscation task and provide:
1. Recommended transformation techniques
2. Expected impact on binary size and performance
3. Security implications of transformations
4. Validation steps to ensure functionality preservation

Task: {task}

Binary path: {binary_path}

If this is a transformation task, provide detailed instructions for the cunfyooz engine."""
            }
        ]

        try:
            # Call Claude API
            response = self.call_claude(
                messages=messages,
                tools=self.get_tools(),
                max_tokens=self.config.get("max_tokens", 4000)
            )

            # Extract response text
            analysis = ""
            for block in response.content:
                if hasattr(block, 'text'):
                    analysis += block.text

            # If this is a transformation task, execute cunfyooz
            if "obfuscate" in task.lower() or "transform" in task.lower():
                print(f"[CunfyoozAgent] Executing binary transformation...")
                
                # Execute cunfyooz transformation
                result = self._execute_cunfyooz_transformation(binary_path)
                
                if result["status"] == "success":
                    print(f"[CunfyoozAgent] Transformation completed successfully")
                    
                    # Update analysis with transformation results
                    analysis += f"\n\nTransformation Results:\n{result['output']}"
                    
                    # Save transformed binary as artifact
                    self.save_artifact(result['transformed_path'], "transformed_binary")
                    
                    return {
                        "status": "success",
                        "findings": analysis,
                        "artifacts": self.artifacts,
                        "metadata": {
                            "task": task,
                            "original_binary": binary_path,
                            "transformed_binary": result['transformed_path'],
                            "model": self.config.get("model"),
                            "tokens_used": response.usage.output_tokens if hasattr(response, 'usage') else 0
                        }
                    }
                else:
                    print(f"[CunfyoozAgent] Transformation failed: {result['error']}")
                    return {
                        "status": "error",
                        "error": f"Transformation failed: {result['error']}",
                        "findings": analysis,
                        "artifacts": self.artifacts
                    }
            else:
                # Just analysis task
                print(f"[CunfyoozAgent] Analysis completed successfully")
                
                return {
                    "status": "success",
                    "findings": analysis,
                    "artifacts": self.artifacts,
                    "metadata": {
                        "task": task,
                        "model": self.config.get("model"),
                        "tokens_used": response.usage.output_tokens if hasattr(response, 'usage') else 0
                    }
                }

        except Exception as e:
            print(f"[CunfyoozAgent] Error: {str(e)}")
            return {
                "status": "error",
                "error": str(e),
                "findings": None,
                "artifacts": []
            }

    def _extract_binary_path(self, task: str) -> Optional[str]:
        """Extract binary path from task description"""
        # Look for common patterns in the task that indicate a binary path
        import re
        
        # Pattern 1: "obfuscate binary at path/to/file.exe"
        pattern1 = r'(?:obfuscate|transform|analyze)\s+binary\s+at\s+([^\s]+)'
        match = re.search(pattern1, task, re.IGNORECASE)
        if match:
            return match.group(1)
        
        # Pattern 2: Just a path mentioned in the task
        # Look for common binary extensions
        for ext in ['.exe', '.dll', '.bin', '.out']:
            if ext in task.lower():
                # Extract potential path
                path_candidates = re.findall(r'([^\s]*\.(?:exe|dll|bin|out))', task, re.IGNORECASE)
                if path_candidates:
                    return path_candidates[0]
        
        return None

    def _execute_cunfyooz_transformation(self, binary_path: str) -> Dict[str, Any]:
        """Execute cunfyooz transformation on the specified binary"""
        try:
            # Check if cunfyooz binary exists
            cunfyooz_path = "./bin/cunfyooz"
            if not os.path.exists(cunfyooz_path):
                # Try to find it in the project root
                cunfyooz_path = "/home/mrnob0dy666/cunfyooz/bin/cunfyooz"
                if not os.path.exists(cunfyooz_path):
                    return {
                        "status": "error",
                        "error": "cunfyooz binary not found. Please build the cunfyooz project first."
                    }
            
            # Execute cunfyooz transformation
            cmd = [cunfyooz_path, binary_path]
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
                    return {
                        "status": "success",
                        "output": result.stdout,
                        "transformed_path": transformed_path,
                        "error": result.stderr if result.stderr else None
                    }
                else:
                    return {
                        "status": "error",
                        "error": f"Transformed binary not found at expected location: {transformed_path}"
                    }
            else:
                return {
                    "status": "error",
                    "error": f"cunfyooz execution failed with return code {result.returncode}: {result.stderr}"
                }
                
        except subprocess.TimeoutExpired:
            return {
                "status": "error",
                "error": "cunfyooz transformation timed out after 5 minutes"
            }
        except Exception as e:
            return {
                "status": "error",
                "error": f"Error executing cunfyooz: {str(e)}"
            }


class CunfyoozAnalysisAgent(BaseAgent):
    """
    Specialized agent for analyzing binary properties and transformation effectiveness.
    """
    
    def __init__(self, config: Dict[str, Any]):
        super().__init__(
            agent_id="cunfyooz_analysis_agent",
            name="Cunfyooz Analysis Agent",
            description="Analyzes binary properties and evaluates transformation effectiveness",
            capabilities=[
                "Binary structure analysis",
                "Size comparison",
                "Entropy analysis",
                "Signature detection",
                "Transformation effectiveness evaluation"
            ],
            config=config
        )

    def get_tools(self) -> List[Dict[str, Any]]:
        """Define tools this agent can use"""
        return [
            ToolDefinitions.file_read(),
            ToolDefinitions.file_write(),
            ToolDefinitions.run_command(),
            ToolDefinitions.json_load(),
            ToolDefinitions.json_save(),
        ]

    def run(self, task: str, context: Optional[Dict[str, Any]] = None) -> Dict[str, Any]:
        """
        Execute binary analysis task.

        Args:
            task: Analysis task description
            context: Optional context from previous agents (e.g., paths to original and transformed binaries)

        Returns:
            Analysis results and insights
        """
        print(f"[CunfyoozAnalysisAgent] Starting binary analysis: {task}")

        # Extract binary paths from context or task
        original_binary = None
        transformed_binary = None
        
        if context:
            # Look for binary paths in context
            if "original_binary" in context:
                original_binary = context["original_binary"]
            elif "previous_stage" in context and "original_binary" in context["previous_stage"]:
                original_binary = context["previous_stage"]["original_binary"]
                
            if "transformed_binary" in context:
                transformed_binary = context["transformed_binary"]
            elif "previous_stage" in context and "transformed_binary" in context["previous_stage"]:
                transformed_binary = context["previous_stage"]["transformed_binary"]

        # Build messages for Claude
        messages = [
            {
                "role": "user",
                "content": f"""Analyze the following binary transformation and provide:
1. Size comparison between original and transformed binaries
2. Structural differences
3. Entropy analysis
4. Effectiveness of obfuscation techniques
5. Potential security improvements

Analysis task: {task}

Original binary: {original_binary or 'Not provided'}
Transformed binary: {transformed_binary or 'Not provided'}

If binary paths are provided, analyze the actual files. Otherwise, provide general analysis based on the task description."""
            }
        ]

        try:
            # Call Claude API
            response = self.call_claude(
                messages=messages,
                tools=self.get_tools(),
                max_tokens=self.config.get("max_tokens", 4000)
            )

            # Extract response text
            analysis = ""
            for block in response.content:
                if hasattr(block, 'text'):
                    analysis += block.text

            # Save as artifact
            self.save_artifact(analysis, "analysis_report")

            print(f"[CunfyoozAnalysisAgent] Analysis completed successfully")

            return {
                "status": "success",
                "findings": analysis,
                "artifacts": self.artifacts,
                "metadata": {
                    "task": task,
                    "original_binary": original_binary,
                    "transformed_binary": transformed_binary,
                    "model": self.config.get("model"),
                    "tokens_used": response.usage.output_tokens if hasattr(response, 'usage') else 0
                }
            }

        except Exception as e:
            print(f"[CunfyoozAnalysisAgent] Error: {str(e)}")
            return {
                "status": "error",
                "error": str(e),
                "findings": None,
                "artifacts": []
            }