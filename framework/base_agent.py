"""
Base Agent Framework for Claude API
Core abstract class that all agents inherit from.
"""
from abc import ABC, abstractmethod
from typing import Dict, List, Optional, Any
from enum import Enum
import os
import anthropic
from datetime import datetime


class AgentStatus(Enum):
    """Agent execution states"""
    IDLE = "idle"
    RUNNING = "running"
    COMPLETED = "completed"
    FAILED = "failed"


class BaseAgent(ABC):
    """
    Abstract base class for Claude API agents.

    Provides:
    - Claude API client setup and interaction
    - State management (status, artifacts, results)
    - Tool system integration
    - Memory persistence hooks
    - Standard execution lifecycle

    Subclasses must implement:
    - run(task, context): Main agent logic
    - get_tools(): Return list of Claude API tool definitions (optional)
    """

    def __init__(
        self,
        agent_id: str,
        name: str,
        description: str,
        capabilities: List[str],
        config: Dict[str, Any]
    ):
        self.agent_id = agent_id
        self.name = name
        self.description = description
        self.capabilities = capabilities
        self.config = config

        # State management
        self.status = AgentStatus.IDLE
        self.artifacts = []
        self.results = {}
        self.start_time = None
        self.end_time = None

        # Initialize Claude API client
        self.client = self._setup_anthropic_client()

    def _setup_anthropic_client(self) -> anthropic.Anthropic:
        """Initialize Anthropic client with API key from environment"""
        api_key = os.getenv('ANTHROPIC_API_KEY')
        if not api_key:
            raise ValueError(
                "ANTHROPIC_API_KEY environment variable not set. "
                "Get your API key from: https://console.anthropic.com/"
            )
        return anthropic.Anthropic(api_key=api_key)

    def call_claude(
        self,
        messages: List[Dict[str, Any]],
        tools: Optional[List[Dict[str, Any]]] = None,
        max_tokens: int = 4000,
        temperature: float = 1.0
    ) -> Any:
        """
        Call Claude API with messages and optional tools.

        Args:
            messages: List of message dicts with 'role' and 'content'
            tools: Optional list of tool definitions for Claude
            max_tokens: Maximum tokens in response
            temperature: Sampling temperature (0.0 to 1.0)

        Returns:
            Claude API response object
        """
        model = self.config.get("model", "claude-sonnet-4-5-20250929")

        request_params = {
            "model": model,
            "max_tokens": max_tokens,
            "temperature": temperature,
            "messages": messages
        }

        if tools:
            request_params["tools"] = tools

        response = self.client.messages.create(**request_params)
        return response

    @abstractmethod
    def run(self, task: str, context: Optional[Dict[str, Any]] = None) -> Dict[str, Any]:
        """
        Execute agent task. Must be implemented by subclasses.

        Args:
            task: Task description or instruction
            context: Optional context data from previous agents/stages

        Returns:
            Result dictionary with:
            - status: "success" or "error"
            - findings: Main results/output
            - artifacts: List of generated artifacts
            - metadata: Additional information
        """
        pass

    def get_tools(self) -> List[Dict[str, Any]]:
        """
        Return Claude API tool definitions for this agent.
        Override to provide agent-specific tools.

        Returns:
            List of tool definition dicts in Claude API format
        """
        return []

    # Lifecycle management methods

    def start(self) -> None:
        """Mark agent as running"""
        self.status = AgentStatus.RUNNING
        self.start_time = datetime.now()
        self.artifacts = []
        self.results = {}

    def complete(self, results: Dict[str, Any]) -> None:
        """Mark agent as completed with results"""
        self.status = AgentStatus.COMPLETED
        self.end_time = datetime.now()
        self.results = results

    def fail(self, error: str) -> None:
        """Mark agent as failed with error message"""
        self.status = AgentStatus.FAILED
        self.end_time = datetime.now()
        self.results = {"error": error}

    def save_artifact(self, artifact_data: Any, artifact_type: str) -> None:
        """
        Save an artifact produced by this agent.

        Args:
            artifact_data: The artifact content
            artifact_type: Type/category of artifact
        """
        artifact = {
            "type": artifact_type,
            "data": artifact_data,
            "timestamp": datetime.now().isoformat()
        }
        self.artifacts.append(artifact)

    # Memory hooks (override to add persistence)

    def store_in_memory(self, key: str, value: Any, category: str = "general") -> None:
        """Hook for storing data in persistent memory"""
        pass

    def retrieve_from_memory(self, key: str, category: str = "general") -> Optional[Any]:
        """Hook for retrieving data from persistent memory"""
        return None

    # Utility methods

    def get_execution_time(self) -> Optional[float]:
        """Get execution duration in seconds"""
        if self.start_time and self.end_time:
            return (self.end_time - self.start_time).total_seconds()
        return None

    def __repr__(self) -> str:
        return f"<{self.__class__.__name__}(id={self.agent_id}, status={self.status.value})>"
