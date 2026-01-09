"""
Example 2: Cunfyooz Multi-Agent Swarm

Demonstrates:
- Running multiple agents in parallel (swarm mode)
- Cunfyooz agent working alongside analysis agent
- Result aggregation from multiple agents
"""
import sys
from pathlib import Path

# Add parent directory to path for imports
sys.path.insert(0, str(Path(__file__).parent.parent))

from agents.cunfyooz_agent import CunfyoozAgent, CunfyoozAnalysisAgent
from framework import AgentOrchestrator


def main():
    """Run a swarm of agents with cunfyooz and analysis capabilities"""

    # 1. Create agent configuration
    config = {
        "model": "claude-sonnet-4-5-20250929",
        "max_tokens": 4000,
        "temperature": 1.0
    }

    # 2. Initialize orchestrator
    print("Initializing Agent Orchestrator...")
    orchestrator = AgentOrchestrator({"max_concurrent_agents": 3})

    # 3. Initialize agents
    print("Initializing agents...")
    cunfyooz_agent = CunfyoozAgent(config)
    analysis_agent = CunfyoozAnalysisAgent(config)

    # 4. Register agents with orchestrator
    orchestrator.register_agent("cunfyooz", cunfyooz_agent)
    orchestrator.register_agent("analysis", analysis_agent)

    # 5. Define task
    task = "perform binary obfuscation and analysis on ./test_binary.exe"

    # 6. Run swarm
    print(f"\nRunning swarm with task: {task}\n")
    print("Agents participating: cunfyooz, analysis")
    result = orchestrator.run_swarm(
        task=task,
        agent_ids=["cunfyooz", "analysis"]
    )

    # 7. Display results
    print("\n" + "=" * 60)
    print("SWARM EXECUTION RESULTS")
    print("=" * 60)
    print(f"Agents run: {result['agents_run']}")
    print(f"Successful: {result['successful']}")
    print(f"Failed: {result['failed']}")

    print("\nIndividual Results:")
    for agent_id, agent_result in result['results'].items():
        print(f"\n--- {agent_id.upper()} AGENT ---")
        print(f"Status: {agent_result['status']}")
        if agent_result['status'] == 'success':
            print(f"Findings preview: {str(agent_result['findings'])[:200]}...")
            print(f"Artifacts: {len(agent_result.get('artifacts', []))}")
        else:
            print(f"Error: {agent_result.get('error', 'Unknown error')}")


if __name__ == "__main__":
    # Ensure ANTHROPIC_API_KEY is set in environment
    import os
    if not os.getenv("ANTHROPIC_API_KEY"):
        print("WARNING: ANTHROPIC_API_KEY environment variable not set")
        print("This is needed for Claude API access")
        print("Set it with: export ANTHROPIC_API_KEY='your-api-key'")
        print("Continuing anyway for testing purposes...")

    main()