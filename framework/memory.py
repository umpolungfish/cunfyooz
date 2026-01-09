"""
Agent Memory System
Provides persistent storage for agent state and data.
"""
from typing import Dict, Any, Optional
import json
import os
from pathlib import Path
from datetime import datetime


class AgentMemory:
    """
    Persistent memory system for agents.

    Features:
    - Category-based storage (general, updates, sessions)
    - JSON-based persistence
    - Session tracking
    - Event logging
    """

    def __init__(self, agent_id: str, storage_dir: str = ".agent_memory"):
        """
        Initialize agent memory.

        Args:
            agent_id: Unique agent identifier
            storage_dir: Directory for memory storage
        """
        self.agent_id = agent_id
        self.storage_dir = Path(storage_dir)
        self.storage_dir.mkdir(parents=True, exist_ok=True)

        self.memory_file = self.storage_dir / f"{agent_id}_memory.json"
        self.memory = self._load_memory()

    def _load_memory(self) -> Dict[str, Any]:
        """Load memory from disk"""
        if self.memory_file.exists():
            try:
                with open(self.memory_file, 'r') as f:
                    return json.load(f)
            except Exception as e:
                print(f"Error loading memory: {e}")
                return self._init_memory_structure()
        return self._init_memory_structure()

    def _init_memory_structure(self) -> Dict[str, Any]:
        """Initialize empty memory structure"""
        return {
            "agent_id": self.agent_id,
            "created_at": datetime.now().isoformat(),
            "last_updated": datetime.now().isoformat(),
            "categories": {
                "general": {},
                "updates": [],
                "sessions": []
            }
        }

    def _save_memory(self) -> None:
        """Save memory to disk"""
        self.memory["last_updated"] = datetime.now().isoformat()
        try:
            with open(self.memory_file, 'w') as f:
                json.dump(self.memory, f, indent=2)
        except Exception as e:
            print(f"Error saving memory: {e}")

    def store(self, key: str, value: Any, category: str = "general") -> None:
        """
        Store a value in memory.

        Args:
            key: Storage key
            value: Value to store
            category: Category (general, updates, sessions)
        """
        if category not in self.memory["categories"]:
            self.memory["categories"][category] = {}

        self.memory["categories"][category][key] = value
        self._save_memory()

    def retrieve(self, key: str, category: str = "general") -> Optional[Any]:
        """
        Retrieve a value from memory.

        Args:
            key: Storage key
            category: Category to retrieve from

        Returns:
            Stored value or None if not found
        """
        return self.memory["categories"].get(category, {}).get(key)

    def delete(self, key: str, category: str = "general") -> bool:
        """
        Delete a value from memory.

        Args:
            key: Storage key
            category: Category to delete from

        Returns:
            True if deleted, False if not found
        """
        if category in self.memory["categories"]:
            if key in self.memory["categories"][category]:
                del self.memory["categories"][category][key]
                self._save_memory()
                return True
        return False

    def add_update(self, update: str) -> None:
        """Add an update to the updates list"""
        self.memory["categories"]["updates"].append({
            "timestamp": datetime.now().isoformat(),
            "update": update
        })
        self._save_memory()

    def start_session(self) -> str:
        """
        Start a new session.

        Returns:
            Session ID
        """
        session_id = f"session_{len(self.memory['categories']['sessions']) + 1}"
        session = {
            "session_id": session_id,
            "start_time": datetime.now().isoformat(),
            "events": []
        }
        self.memory["categories"]["sessions"].append(session)
        self._save_memory()
        return session_id

    def log_event(self, session_id: str, event: str, data: Any = None) -> None:
        """
        Log an event to a session.

        Args:
            session_id: Session to log to
            event: Event description
            data: Optional event data
        """
        for session in self.memory["categories"]["sessions"]:
            if session["session_id"] == session_id:
                session["events"].append({
                    "timestamp": datetime.now().isoformat(),
                    "event": event,
                    "data": data
                })
                self._save_memory()
                return

    def end_session(self, session_id: str) -> None:
        """
        End a session.

        Args:
            session_id: Session to end
        """
        for session in self.memory["categories"]["sessions"]:
            if session["session_id"] == session_id:
                session["end_time"] = datetime.now().isoformat()
                self._save_memory()
                return

    def get_all_sessions(self) -> list:
        """Get all sessions"""
        return self.memory["categories"]["sessions"]

    def clear_category(self, category: str) -> None:
        """Clear all data in a category"""
        if category in self.memory["categories"]:
            if isinstance(self.memory["categories"][category], dict):
                self.memory["categories"][category] = {}
            elif isinstance(self.memory["categories"][category], list):
                self.memory["categories"][category] = []
            self._save_memory()

    def clear_all(self) -> None:
        """Clear all memory"""
        self.memory = self._init_memory_structure()
        self._save_memory()
