"""
Inter-Agent Communication System
Enables agents to send messages and collaborate.
"""
from typing import Dict, List, Any, Optional
from dataclasses import dataclass, asdict
from datetime import datetime
from pathlib import Path
import json
from enum import Enum


class MessageType(Enum):
    """Types of inter-agent messages"""
    REQUEST = "request"
    RESPONSE = "response"
    NOTIFICATION = "notification"
    COLLABORATION = "collaboration_request"


@dataclass
class Message:
    """
    Inter-agent message.

    Attributes:
        message_id: Unique message identifier
        from_agent: Sending agent ID
        to_agent: Receiving agent ID (or "broadcast")
        message_type: Type of message
        content: Message content
        priority: Priority level (1-10, 10 is highest)
        timestamp: When message was created
        metadata: Additional metadata
    """
    message_id: str
    from_agent: str
    to_agent: str
    message_type: MessageType
    content: str
    priority: int = 5
    timestamp: str = None
    metadata: Optional[Dict[str, Any]] = None

    def __post_init__(self):
        if self.timestamp is None:
            self.timestamp = datetime.now().isoformat()
        if self.metadata is None:
            self.metadata = {}


class AgentCommunication:
    """
    Manages message passing between agents.

    Features:
    - Send/receive messages
    - Message queuing and priority
    - Broadcast capability
    - Collaboration requests
    """

    def __init__(self, storage_dir: str = ".agent_messages"):
        """
        Initialize communication system.

        Args:
            storage_dir: Directory for message storage
        """
        self.storage_dir = Path(storage_dir)
        self.storage_dir.mkdir(parents=True, exist_ok=True)
        self.message_counter = 0

    def _get_inbox_path(self, agent_id: str) -> Path:
        """Get inbox file path for an agent"""
        return self.storage_dir / f"{agent_id}_inbox.json"

    def _get_outbox_path(self, agent_id: str) -> Path:
        """Get outbox file path for an agent"""
        return self.storage_dir / f"{agent_id}_outbox.json"

    def _load_messages(self, file_path: Path) -> List[Dict[str, Any]]:
        """Load messages from file"""
        if file_path.exists():
            try:
                with open(file_path, 'r') as f:
                    return json.load(f)
            except Exception:
                return []
        return []

    def _save_messages(self, file_path: Path, messages: List[Dict[str, Any]]) -> None:
        """Save messages to file"""
        try:
            with open(file_path, 'w') as f:
                json.dump(messages, f, indent=2)
        except Exception as e:
            print(f"Error saving messages: {e}")

    def send_message(
        self,
        from_agent: str,
        to_agent: str,
        content: str,
        message_type: MessageType = MessageType.NOTIFICATION,
        priority: int = 5,
        metadata: Optional[Dict[str, Any]] = None
    ) -> str:
        """
        Send a message from one agent to another.

        Args:
            from_agent: Sender agent ID
            to_agent: Receiver agent ID (or "broadcast" for all)
            content: Message content
            message_type: Type of message
            priority: Priority level (1-10)
            metadata: Additional metadata

        Returns:
            Message ID
        """
        self.message_counter += 1
        message_id = f"msg_{self.message_counter}_{datetime.now().timestamp()}"

        message = Message(
            message_id=message_id,
            from_agent=from_agent,
            to_agent=to_agent,
            message_type=message_type,
            content=content,
            priority=priority,
            metadata=metadata
        )

        # Save to sender's outbox
        outbox_path = self._get_outbox_path(from_agent)
        outbox = self._load_messages(outbox_path)
        outbox.append(asdict(message))
        self._save_messages(outbox_path, outbox)

        # Save to receiver's inbox (or all inboxes for broadcast)
        if to_agent == "broadcast":
            # In a real implementation, you'd iterate over all known agents
            pass
        else:
            inbox_path = self._get_inbox_path(to_agent)
            inbox = self._load_messages(inbox_path)
            inbox.append(asdict(message))
            self._save_messages(inbox_path, inbox)

        return message_id

    def receive_messages(
        self,
        agent_id: str,
        unread_only: bool = True
    ) -> List[Dict[str, Any]]:
        """
        Receive messages for an agent.

        Args:
            agent_id: Agent to receive messages for
            unread_only: Only return unread messages

        Returns:
            List of messages
        """
        inbox_path = self._get_inbox_path(agent_id)
        messages = self._load_messages(inbox_path)

        if unread_only:
            # Filter to unread messages
            messages = [m for m in messages if not m.get("read", False)]

        # Sort by priority (highest first) and timestamp
        messages.sort(key=lambda m: (-m.get("priority", 5), m.get("timestamp", "")))

        return messages

    def mark_as_read(self, agent_id: str, message_id: str) -> bool:
        """
        Mark a message as read.

        Args:
            agent_id: Agent ID
            message_id: Message ID to mark as read

        Returns:
            True if marked, False if not found
        """
        inbox_path = self._get_inbox_path(agent_id)
        messages = self._load_messages(inbox_path)

        for message in messages:
            if message["message_id"] == message_id:
                message["read"] = True
                self._save_messages(inbox_path, messages)
                return True

        return False

    def send_collaboration_request(
        self,
        from_agent: str,
        to_agent: str,
        task: str,
        context: Optional[Dict[str, Any]] = None
    ) -> str:
        """
        Send a collaboration request to another agent.

        Args:
            from_agent: Requesting agent
            to_agent: Target agent
            task: Task description
            context: Optional context data

        Returns:
            Message ID
        """
        metadata = {
            "type": "collaboration_request",
            "task": task,
            "context": context or {}
        }

        return self.send_message(
            from_agent=from_agent,
            to_agent=to_agent,
            content=f"Collaboration request: {task}",
            message_type=MessageType.COLLABORATION,
            priority=7,
            metadata=metadata
        )

    def send_response(
        self,
        from_agent: str,
        to_agent: str,
        original_message_id: str,
        response_content: str,
        response_data: Optional[Dict[str, Any]] = None
    ) -> str:
        """
        Send a response to a previous message.

        Args:
            from_agent: Responding agent
            to_agent: Original sender
            original_message_id: ID of message being responded to
            response_content: Response content
            response_data: Optional response data

        Returns:
            Message ID
        """
        metadata = {
            "in_response_to": original_message_id,
            "response_data": response_data or {}
        }

        return self.send_message(
            from_agent=from_agent,
            to_agent=to_agent,
            content=response_content,
            message_type=MessageType.RESPONSE,
            priority=6,
            metadata=metadata
        )

    def get_conversation(
        self,
        agent_id: str,
        other_agent_id: str
    ) -> List[Dict[str, Any]]:
        """
        Get all messages in a conversation between two agents.

        Args:
            agent_id: First agent
            other_agent_id: Second agent

        Returns:
            List of messages sorted by timestamp
        """
        inbox = self._load_messages(self._get_inbox_path(agent_id))
        outbox = self._load_messages(self._get_outbox_path(agent_id))

        # Filter messages between these two agents
        conversation = []
        for msg in inbox:
            if msg["from_agent"] == other_agent_id:
                conversation.append(msg)
        for msg in outbox:
            if msg["to_agent"] == other_agent_id:
                conversation.append(msg)

        # Sort by timestamp
        conversation.sort(key=lambda m: m.get("timestamp", ""))

        return conversation
