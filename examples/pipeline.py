"""
Example 3: Cunfyooz Agent Pipeline

Demonstrates:
- Sequential execution of agents in a pipeline
- Context passing between cunfyooz transformation and analysis stages
- Multi-stage binary processing workflow
"""
import sys
from pathlib import Path

# Add parent directory to path for imports
sys.path.insert(0, str(Path(__file__).parent.parent))

from agents.cunfyooz_agent import CunfyoozAgent, CunfyoozAnalysisAgent
from framework import AgentOrchestrator


def main():
    """Run a pipeline of agents for comprehensive binary processing"""

    # 1. Create agent configuration
    config = {
        "model": "claude-sonnet-4-5-20250929",
        "max_tokens": 4000,
        "temperature": 1.0
    }

    # 2. Initialize orchestrator
    print("Initializing Agent Orchestrator...")
    orchestrator = AgentOrchestrator()

    # 3. Initialize agents
    print("Initializing agents...")
    cunfyooz_agent = CunfyoozAgent(config)
    analysis_agent = CunfyoozAnalysisAgent(config)

    # 4. Register agents with orchestrator
    orchestrator.register_agent("cunfyooz", cunfyooz_agent)
    orchestrator.register_agent("analysis", analysis_agent)

    # 5. Define base task
    task = "obfuscate and analyze binary ./test_binary.exe"

    # 6. Define initial context
    initial_context = {
        "binary_path": "./test_binary.exe",
        "transformation_goal": "increase entropy and obfuscate control flow"
    }

    # 7. Run pipeline - cunfyooz transforms, then analysis evaluates
    print(f"\nRunning pipeline with task: {task}")
    print("Pipeline stages: [cunfyooz transformation] -> [analysis]")
    result = orchestrator.run_pipeline(
        task=task,
        agent_ids=["cunfyooz", "analysis"],
        initial_context=initial_context
    )

    # 8. Display results
    print("\n" + "=" * 60)
    print("PIPELINE EXECUTION RESULTS")
    print("=" * 60)
    
    if result['status'] == 'success':
        print(f"Stages completed: {result['stages_completed']}")
        
        print("\nStage-by-stage results:")
        for stage in result['pipeline_results']:
            agent_id = stage['agent_id']
            stage_result = stage['result']
            
            print(f"\n--- STAGE {stage['stage']}: {agent_id.upper()} ---")
            print(f"Status: {stage_result['status']}")
            
            if stage_result['status'] == 'success':
                findings_preview = str(stage_result['findings'])[:200] + "..." if len(str(stage_result['findings'])) > 200 else str(stage_result['findings'])
                print(f"Findings preview: {findings_preview}")
                print(f"Artifacts: {len(stage_result.get('artifacts', []))}")
            else:
                print(f"Error: {stage_result.get('error', 'Unknown error')}")
        
        print(f"\nFinal context: {result['final_context']}")
    else:
        print(f"Pipeline failed at stage {result['failed_at_stage']}")
        for stage in result['pipeline_results']:
            print(f"Stage {stage['stage']} ({stage['agent_id']}): {stage['result']['status']}")


if __name__ == "__main__":
    # Ensure ANTHROPIC_API_KEY is set in environment
    import os
    if not os.getenv("ANTHROPIC_API_KEY"):
        print("WARNING: ANTHROPIC_API_KEY environment variable not set")
        print("This is needed for Claude API access")
        print("Set it with: export ANTHROPIC_API_KEY='your-api-key'")
        print("Continuing anyway for testing purposes...")

    main()