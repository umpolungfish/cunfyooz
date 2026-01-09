"""
Example 1: Single Cunfyooz Agent Execution

Demonstrates:
- Basic cunfyooz agent setup
- Running a single agent for binary transformation
- Accessing transformation results
"""
import sys
from pathlib import Path

# Add parent directory to path for imports
sys.path.insert(0, str(Path(__file__).parent.parent))

from agents.cunfyooz_agent import CunfyoozAgent


def main():
    """Run a single cunfyooz agent with a binary transformation task"""

    # 1. Create agent configuration
    config = {
        "model": "claude-sonnet-4-5-20250929",
        "max_tokens": 4000,
        "temperature": 1.0
    }

    # 2. Initialize cunfyooz agent
    print("Initializing Cunfyooz Agent...")
    agent = CunfyoozAgent(config)

    # 3. Define task - this would typically point to an actual binary file
    task = "analyze and obfuscate binary at ./test_binary.exe"

    # 4. Run agent
    print(f"\nRunning task: {task}\n")
    result = agent.run(task)

    # 5. Display results
    print("\n" + "=" * 60)
    print("CUNFYOOZ AGENT RESULTS")
    print("=" * 60)
    print(f"Status: {result['status']}")
    
    if result['status'] == 'success':
        print(f"\nFindings:\n{result['findings']}")
        print(f"\nArtifacts generated: {len(result['artifacts'])}")
        for i, artifact in enumerate(result['artifacts']):
            print(f"  Artifact {i+1}: {artifact['type']} ({len(str(artifact['data']))} chars)")
        
        print(f"\nMetadata: {result.get('metadata', {})}")
    else:
        print(f"\nError: {result.get('error', 'Unknown error')}")


if __name__ == "__main__":
    # Ensure ANTHROPIC_API_KEY is set in environment
    import os
    if not os.getenv("ANTHROPIC_API_KEY"):
        print("WARNING: ANTHROPIC_API_KEY environment variable not set")
        print("This is needed for Claude API access")
        print("Set it with: export ANTHROPIC_API_KEY='your-api-key'")
        print("Continuing anyway for testing purposes...")

    main()