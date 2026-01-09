"""
Cunfyooz Agent Framework
A specialized framework for binary obfuscation and metamorphic transformations.
"""

from .base_agent import BaseAgent, AgentStatus
from .orchestrator import AgentOrchestrator
from .tools import ToolDefinitions, ToolExecutor
from .memory import AgentMemory
from .communication import AgentCommunication, Message, MessageType
from .binary_tools import BinaryToolDefinitions, BinaryToolExecutor

__version__ = "1.0.0"

__all__ = [
    "BaseAgent",
    "AgentStatus",
    "AgentOrchestrator",
    "ToolDefinitions",
    "ToolExecutor",
    "AgentMemory",
    "AgentCommunication",
    "Message",
    "MessageType",
    "BinaryToolDefinitions",
    "BinaryToolExecutor"
]
