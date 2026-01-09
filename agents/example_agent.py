"""
Example Agent Implementations
Demonstrates how to create custom agents using the framework.
"""
from typing import Dict, List, Any, Optional
import sys
from pathlib import Path

# Add parent directory to path for imports
sys.path.insert(0, str(Path(__file__).parent.parent))

from framework import BaseAgent, ToolDefinitions


class ResearchAgent(BaseAgent):
    """
    Example: Research agent that gathers and analyzes information.

    Demonstrates:
    - Basic agent implementation
    - Using Claude API for research tasks
    - Tool integration
    - Artifact generation
    """

    def __init__(self, config: Dict[str, Any]):
        super().__init__(
            agent_id="research_agent",
            name="Research Agent",
            description="Gathers and analyzes information on given topics",
            capabilities=[
                "Web research",
                "Information synthesis",
                "Source analysis",
                "Report generation"
            ],
            config=config
        )

    def get_tools(self) -> List[Dict[str, Any]]:
        """Define tools this agent can use"""
        return [
            ToolDefinitions.web_fetch(),
            ToolDefinitions.file_read(),
            ToolDefinitions.file_write(),
        ]

    def run(self, task: str, context: Optional[Dict[str, Any]] = None) -> Dict[str, Any]:
        """
        Execute research task.

        Args:
            task: Research topic or question
            context: Optional context from previous agents

        Returns:
            Research findings and generated artifacts
        """
        print(f"[ResearchAgent] Starting research on: {task}")

        # Build system prompt
        system_prompt = f"""You are a research agent specialized in gathering and analyzing information.
Your capabilities: {', '.join(self.capabilities)}

Task: {task}
"""

        if context:
            system_prompt += f"\nContext from previous agents:\n{context}"

        # Build messages for Claude
        messages = [
            {
                "role": "user",
                "content": f"""Research the following topic and provide:
1. Key findings
2. Important sources
3. Summary analysis

Topic: {task}

Provide structured output with clear sections."""
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
            findings = ""
            for block in response.content:
                if hasattr(block, 'text'):
                    findings += block.text

            # Save as artifact
            self.save_artifact(findings, "research_report")

            print(f"[ResearchAgent] Research completed successfully")

            return {
                "status": "success",
                "findings": findings,
                "artifacts": self.artifacts,
                "metadata": {
                    "task": task,
                    "model": self.config.get("model"),
                    "tokens_used": response.usage.output_tokens
                }
            }

        except Exception as e:
            print(f"[ResearchAgent] Error: {str(e)}")
            return {
                "status": "error",
                "error": str(e),
                "findings": None,
                "artifacts": []
            }


class AnalysisAgent(BaseAgent):
    """
    Example: Analysis agent that processes and interprets data.

    Demonstrates:
    - Processing context from previous agents
    - Advanced tool usage
    - Multi-step analysis
    - Structured output generation
    """

    def __init__(self, config: Dict[str, Any]):
        super().__init__(
            agent_id="analysis_agent",
            name="Analysis Agent",
            description="Analyzes data and generates insights",
            capabilities=[
                "Data analysis",
                "Pattern recognition",
                "Insight generation",
                "Recommendation synthesis"
            ],
            config=config
        )

    def get_tools(self) -> List[Dict[str, Any]]:
        """Define tools this agent can use"""
        return [
            ToolDefinitions.file_read(),
            ToolDefinitions.file_write(),
            ToolDefinitions.json_load(),
            ToolDefinitions.json_save(),
        ]

    def run(self, task: str, context: Optional[Dict[str, Any]] = None) -> Dict[str, Any]:
        """
        Execute analysis task.

        Args:
            task: Analysis objective
            context: Data/findings from previous agents

        Returns:
            Analysis results and insights
        """
        print(f"[AnalysisAgent] Starting analysis: {task}")

        # Extract previous findings if available
        previous_findings = ""
        if context and "previous_stage" in context:
            prev = context["previous_stage"]
            previous_findings = prev.get("findings", "")

        # Build messages for Claude
        messages = [
            {
                "role": "user",
                "content": f"""Analyze the following information and provide:
1. Key insights
2. Patterns identified
3. Actionable recommendations

Analysis objective: {task}

Data to analyze:
{previous_findings if previous_findings else "No previous data provided"}

Provide structured, detailed analysis."""
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

            print(f"[AnalysisAgent] Analysis completed successfully")

            return {
                "status": "success",
                "findings": analysis,
                "artifacts": self.artifacts,
                "metadata": {
                    "task": task,
                    "used_previous_context": bool(previous_findings),
                    "model": self.config.get("model"),
                    "tokens_used": response.usage.output_tokens
                }
            }

        except Exception as e:
            print(f"[AnalysisAgent] Error: {str(e)}")
            return {
                "status": "error",
                "error": str(e),
                "findings": None,
                "artifacts": []
            }


# Template for creating new agents
class CustomAgentTemplate(BaseAgent):
    """
    Template for creating your own custom agent.

    Steps to create a new agent:
    1. Copy this template
    2. Rename the class
    3. Update agent_id, name, description, capabilities
    4. Implement get_tools() if you need custom tools
    5. Implement run() with your agent logic
    """

    def __init__(self, config: Dict[str, Any]):
        super().__init__(
            agent_id="custom_agent",  # Change this
            name="Custom Agent",  # Change this
            description="Description of what this agent does",  # Change this
            capabilities=[
                "Capability 1",
                "Capability 2",
                "Capability 3"
            ],  # Change this
            config=config
        )

    def get_tools(self) -> List[Dict[str, Any]]:
        """Override if your agent needs tools"""
        return ToolDefinitions.get_all_basic_tools()

    def run(self, task: str, context: Optional[Dict[str, Any]] = None) -> Dict[str, Any]:
        """
        Implement your agent logic here.

        Pattern:
        1. Parse task and context
        2. Build messages for Claude
        3. Call self.call_claude()
        4. Process response
        5. Save artifacts with self.save_artifact()
        6. Return result dict
        """
        print(f"[CustomAgent] Starting: {task}")

        # Your implementation here
        messages = [
            {"role": "user", "content": task}
        ]

        try:
            response = self.call_claude(messages=messages)

            # Process response
            result_text = ""
            for block in response.content:
                if hasattr(block, 'text'):
                    result_text += block.text

            return {
                "status": "success",
                "findings": result_text,
                "artifacts": self.artifacts,
                "metadata": {}
            }

        except Exception as e:
            return {
                "status": "error",
                "error": str(e),
                "findings": None,
                "artifacts": []
            }
