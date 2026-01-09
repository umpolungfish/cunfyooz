"""
Tool System for Claude API Agents
Provides common tool definitions that agents can use.
"""
from typing import Dict, List, Any


class ToolDefinitions:
    """
    Common tool definitions in Claude API format.
    Agents can use these pre-built tools or define custom ones.
    """

    @staticmethod
    def file_read() -> Dict[str, Any]:
        """Tool for reading file contents"""
        return {
            "name": "file_read",
            "description": "Read the contents of a file",
            "input_schema": {
                "type": "object",
                "properties": {
                    "path": {
                        "type": "string",
                        "description": "Path to the file to read"
                    }
                },
                "required": ["path"]
            }
        }

    @staticmethod
    def file_write() -> Dict[str, Any]:
        """Tool for writing file contents"""
        return {
            "name": "file_write",
            "description": "Write content to a file",
            "input_schema": {
                "type": "object",
                "properties": {
                    "path": {
                        "type": "string",
                        "description": "Path to the file to write"
                    },
                    "content": {
                        "type": "string",
                        "description": "Content to write to the file"
                    }
                },
                "required": ["path", "content"]
            }
        }

    @staticmethod
    def file_search() -> Dict[str, Any]:
        """Tool for searching files by pattern"""
        return {
            "name": "file_search",
            "description": "Search for files matching a pattern",
            "input_schema": {
                "type": "object",
                "properties": {
                    "pattern": {
                        "type": "string",
                        "description": "File pattern to search for (e.g., *.py, *.js)"
                    },
                    "directory": {
                        "type": "string",
                        "description": "Directory to search in (default: current)"
                    }
                },
                "required": ["pattern"]
            }
        }

    @staticmethod
    def run_command() -> Dict[str, Any]:
        """Tool for executing shell commands"""
        return {
            "name": "run_command",
            "description": "Execute a shell command",
            "input_schema": {
                "type": "object",
                "properties": {
                    "command": {
                        "type": "string",
                        "description": "Shell command to execute"
                    },
                    "timeout": {
                        "type": "integer",
                        "description": "Timeout in seconds (default: 30)"
                    }
                },
                "required": ["command"]
            }
        }

    @staticmethod
    def web_fetch() -> Dict[str, Any]:
        """Tool for fetching web content"""
        return {
            "name": "web_fetch",
            "description": "Fetch content from a URL",
            "input_schema": {
                "type": "object",
                "properties": {
                    "url": {
                        "type": "string",
                        "description": "URL to fetch"
                    },
                    "method": {
                        "type": "string",
                        "description": "HTTP method (default: GET)",
                        "enum": ["GET", "POST", "PUT", "DELETE"]
                    }
                },
                "required": ["url"]
            }
        }

    @staticmethod
    def json_load() -> Dict[str, Any]:
        """Tool for loading JSON files"""
        return {
            "name": "json_load",
            "description": "Load and parse a JSON file",
            "input_schema": {
                "type": "object",
                "properties": {
                    "path": {
                        "type": "string",
                        "description": "Path to JSON file"
                    }
                },
                "required": ["path"]
            }
        }

    @staticmethod
    def json_save() -> Dict[str, Any]:
        """Tool for saving JSON files"""
        return {
            "name": "json_save",
            "description": "Save data as a JSON file",
            "input_schema": {
                "type": "object",
                "properties": {
                    "path": {
                        "type": "string",
                        "description": "Path to save JSON file"
                    },
                    "data": {
                        "type": "object",
                        "description": "Data to save as JSON"
                    }
                },
                "required": ["path", "data"]
            }
        }

    @staticmethod
    def get_all_basic_tools() -> List[Dict[str, Any]]:
        """Get all basic tools as a list"""
        return [
            ToolDefinitions.file_read(),
            ToolDefinitions.file_write(),
            ToolDefinitions.file_search(),
            ToolDefinitions.run_command(),
            ToolDefinitions.web_fetch(),
            ToolDefinitions.json_load(),
            ToolDefinitions.json_save(),
        ]


class ToolExecutor:
    """
    Executes tool calls from Claude API responses.
    Implement handlers for each tool your agents use.
    """

    def __init__(self):
        self.handlers = {}
        self._register_default_handlers()

    def _register_default_handlers(self):
        """Register default tool handlers"""
        # Add your tool execution logic here
        # Example:
        # self.handlers["file_read"] = self._handle_file_read
        pass

    def register_handler(self, tool_name: str, handler_func):
        """Register a custom tool handler"""
        self.handlers[tool_name] = handler_func

    def execute_tool(self, tool_name: str, tool_input: Dict[str, Any]) -> Any:
        """
        Execute a tool call.

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

    # Example handler implementation
    def _handle_file_read(self, tool_input: Dict[str, Any]) -> str:
        """Example: Handle file read tool"""
        path = tool_input["path"]
        try:
            with open(path, 'r') as f:
                return f.read()
        except Exception as e:
            return f"Error reading file: {str(e)}"
