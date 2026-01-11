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
            # If we can't extract from task, try to get it from context
            if context and "binary_path" in context:
                binary_path = context["binary_path"]
            elif context and "input_file" in context:
                binary_path = context["input_file"]
            else:
                return {
                    "status": "error",
                    "error": "No binary path specified in task or context",
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

        # Analyze the binary to extract information for the AI
        binary_info = self._analyze_binary_for_ai(binary_path)

        # Build system prompt
        system_prompt = f"""You are a binary analysis and obfuscation expert.
Your capabilities: {', '.join(self.capabilities)}

Task: {task}

Binary to analyze: {binary_path}

Binary Information:
{binary_info}

Analyze the binary and provide recommendations for obfuscation techniques."""

        if context:
            system_prompt += f"\nContext from previous agents:\n{context}"

        # Build messages for Claude
        messages = [
            {
                "role": "user",
                "content": f"""You are a binary transformation specialist. Based on the following binary information, provide:
1. Recommended transformation techniques based on the binary structure
2. Expected impact on binary size and performance
3. Security implications of transformations
4. Validation steps to ensure functionality preservation

Task: {task}

Binary path: {binary_path}

Binary Information:
{binary_info}

If this is a transformation task, provide detailed instructions for the cunfyooz engine. If this is an analysis task, focus on structural analysis."""
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
            if "obfuscate" in task.lower() or "transform" in task.lower() or "analyze" not in task.lower():
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
                        },
                        "context": {
                            "original_binary": binary_path,
                            "transformed_binary": result['transformed_path']
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

    def _analyze_binary_for_ai(self, binary_path: str) -> str:
        """Analyze binary to extract information for AI analysis"""
        try:
            # Use the wrapper to analyze the binary
            from cunfyooz_wrapper import CunfyoozWrapper
            wrapper = CunfyoozWrapper()

            # Get binary analysis
            analysis = wrapper.analyze_binary(binary_path)

            # Format the analysis for AI consumption
            formatted_analysis = f"""
Binary Analysis Results:
- Path: {analysis.get('path', 'N/A')}
- Size: {analysis.get('size', 'N/A')} bytes
- Type: {analysis.get('type', 'N/A')}
- Entropy: {analysis.get('entropy', 'N/A')}
- Analysis Completed: {analysis.get('analysis_completed', 'N/A')}

Additional Information:
- File extension suggests: {os.path.splitext(binary_path)[1]}
- Last modified: {os.path.getmtime(binary_path)}
"""

            return formatted_analysis

        except Exception as e:
            return f"Could not analyze binary: {str(e)}"

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
            elif "binary_path" in context:
                original_binary = context["binary_path"]
            elif "input_file" in context:
                original_binary = context["input_file"]

            if "transformed_binary" in context:
                transformed_binary = context["transformed_binary"]
            elif "previous_stage" in context and "transformed_binary" in context["previous_stage"]:
                transformed_binary = context["previous_stage"]["transformed_binary"]
            elif "previous_stage" in context and "context" in context["previous_stage"] and "transformed_binary" in context["previous_stage"]["context"]:
                # Check if the previous stage's result has a context with transformed binary
                transformed_binary = context["previous_stage"]["context"]["transformed_binary"]

        # If no original binary found in context, try to extract from task
        if not original_binary:
            # This is a fallback - normally the CLI should pass the binary path in context
            import re
            # Look for common binary extensions in the task
            for ext in ['.exe', '.dll', '.bin', '.out']:
                if ext in task.lower():
                    path_candidates = re.findall(r'([^\s]*\.(?:exe|dll|bin|out))', task, re.IGNORECASE)
                    if path_candidates:
                        original_binary = path_candidates[0]
                        break

        # Analyze the binaries to extract information for the AI
        original_analysis = None
        transformed_analysis = None

        if original_binary and os.path.exists(original_binary):
            original_analysis = self._analyze_binary_for_ai(original_binary)

        if transformed_binary and os.path.exists(transformed_binary):
            transformed_analysis = self._analyze_binary_for_ai(transformed_binary)

        try:
            # Actually analyze the binaries using the wrapper to get real data
            original_analysis_data = None
            transformed_analysis_data = None
            wrapper = None

            if original_binary and os.path.exists(original_binary):
                from cunfyooz_wrapper import CunfyoozWrapper
                wrapper = CunfyoozWrapper()
                original_analysis_data = wrapper.analyze_binary(original_binary)

            if transformed_binary and os.path.exists(transformed_binary):
                from cunfyooz_wrapper import CunfyoozWrapper
                if wrapper is None:
                    wrapper = CunfyoozWrapper()
                transformed_analysis_data = wrapper.analyze_binary(transformed_binary)

            # If we have real analysis data, create a more detailed prompt with actual data
            if original_analysis_data and transformed_analysis_data:
                comparison_data = wrapper.compare_binaries(original_binary, transformed_binary) if wrapper is not None else {}

                detailed_content = f"""Based on the following binary transformation data, provide a detailed analysis:

ORIGINAL BINARY ANALYSIS:
- Path: {original_analysis_data.get('path', 'N/A')}
- Size: {original_analysis_data.get('size', 'N/A')} bytes
- Type: {original_analysis_data.get('type', 'N/A')}
- Entropy: {original_analysis_data.get('entropy', 'N/A')}

TRANSFORMED BINARY ANALYSIS:
- Path: {transformed_analysis_data.get('path', 'N/A')}
- Size: {transformed_analysis_data.get('size', 'N/A')} bytes
- Type: {transformed_analysis_data.get('type', 'N/A')}
- Entropy: {transformed_analysis_data.get('entropy', 'N/A')}

COMPARISON DATA:
- Size Difference: {comparison_data.get('size_difference', 'N/A')} bytes
- Size Change Percent: {comparison_data.get('size_change_percent', 'N/A')}%
- Content Identical: {comparison_data.get('content_identical', 'N/A')}
- Original Hash: {comparison_data.get('original_hash', 'N/A')[:16]}...
- Transformed Hash: {comparison_data.get('transformed_hash', 'N/A')[:16]}...

TASK: Analyze these specific values and provide:
1. Size comparison between original and transformed binaries
2. Structural differences and changes
3. Entropy analysis and statistical differences
4. Effectiveness of obfuscation techniques applied
5. Potential security improvements achieved
6. Validation that functionality is preserved

Provide your analysis based on these actual values, not a statement that you will analyze them."""

                messages = [
                    {
                        "role": "user",
                        "content": detailed_content
                    }
                ]
            # If we don't have real analysis data, use the original prompt
            else:
                messages = [
                    {
                        "role": "user",
                        "content": f"""You are a binary analysis expert specializing in transformation validation. Analyze the following binary transformation and provide:
1. Size comparison between original and transformed binaries
2. Structural differences and changes
3. Entropy analysis and statistical differences
4. Effectiveness of obfuscation techniques applied
5. Potential security improvements achieved
6. Validation that functionality is preserved

Analysis task: {task}

Original binary: {original_binary or 'Not provided'}
Original binary analysis: {original_analysis or 'No analysis available'}

Transformed binary: {transformed_binary or 'Not provided'}
Transformed binary analysis: {transformed_analysis or 'No analysis available'}

Provide a detailed comparative analysis based on the actual binary files and their analysis data."""
                    }
                ]

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

    def _analyze_binary_for_ai(self, binary_path: str) -> str:
        """Analyze binary to extract information for AI analysis"""
        try:
            # Use the wrapper to analyze the binary
            from cunfyooz_wrapper import CunfyoozWrapper
            wrapper = CunfyoozWrapper()

            # Get binary analysis
            analysis = wrapper.analyze_binary(binary_path)

            # Format the analysis for AI consumption
            formatted_analysis = f"""
Binary Analysis Results:
- Path: {analysis.get('path', 'N/A')}
- Size: {analysis.get('size', 'N/A')} bytes
- Type: {analysis.get('type', 'N/A')}
- Entropy: {analysis.get('entropy', 'N/A')}
- Analysis Completed: {analysis.get('analysis_completed', 'N/A')}

Additional Information:
- File extension suggests: {os.path.splitext(binary_path)[1]}
- Last modified: {os.path.getmtime(binary_path)}
"""

            return formatted_analysis

        except Exception as e:
            return f"Could not analyze binary: {str(e)}"