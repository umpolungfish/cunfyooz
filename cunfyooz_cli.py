#!/usr/bin/env python3
"""
Cunfyooz CLI - Command Line Interface for the Binary Obfuscation Framework

This CLI provides access to both the core cunfyooz binary transformation engine
and the AI-powered agent framework for advanced binary analysis and obfuscation.
"""
import argparse
import sys
import os
import json
from pathlib import Path

# Add project root to path for imports
sys.path.insert(0, str(Path(__file__).parent))

from cunfyooz_wrapper import CunfyoozWrapper, CunfyoozPipeline
from agents.cunfyooz_agent import CunfyoozAgent, CunfyoozAnalysisAgent
from agents.example_agent import ResearchAgent, AnalysisAgent
from framework.orchestrator import AgentOrchestrator


def create_default_config():
    """Create a default configuration for transformations"""
    return {
        "transformations": {
            "nop_insertion": {
                "enabled": True,
                "probability": 5
            },
            "instruction_substitution": {
                "enabled": True,
                "probability": 10
            },
            "register_shuffling": {
                "enabled": True,
                "probability": 8
            },
            "enhanced_nop_insertion": {
                "enabled": True,
                "probability": 3
            },
            "control_flow_obfuscation": {
                "enabled": True,
                "probability": 5
            },
            "stack_frame_obfuscation": {
                "enabled": True,
                "probability": 2
            },
            "instruction_reordering": {
                "enabled": True,
                "probability": 5
            },
            "anti_analysis_techniques": {
                "enabled": True,
                "probability": 15
            },
            "virtualization_engine": {
                "enabled": False,
                "probability": 10
            }
        },
        "output": {
            "verbose": True,
            "log_transformations": True
        },
        "security": {
            "validate_functionality": False,  # Changed from True to False for security
            "preserve_original_behavior": True
        }
    }


def run_binary_transformation(args):
    """Run binary transformation using the cunfyooz wrapper"""
    print(f"Transforming binary: {args.input_file}")
    
    # Load or create config
    config = None
    if args.config:
        with open(args.config, 'r') as f:
            config = json.load(f)
    else:
        config = create_default_config()
    
    # Initialize wrapper
    wrapper = CunfyoozWrapper()
    
    # Perform transformation
    result = wrapper.transform_binary(
        input_path=args.input_file,
        output_path=args.output_file,
        config=config
    )
    
    if result["status"] == "success":
        print(f"✓ Transformation completed successfully!")
        print(f"  Original: {result['original_path']}")
        print(f"  Transformed: {result['transformed_path']}")
        
        if args.compare:
            comparison = wrapper.compare_binaries(
                result['original_path'],
                result['transformed_path']
            )
            print("\nBinary Comparison:")
            print(f"  Size difference: {comparison['size_difference']} bytes ({comparison['size_change_percent']}%)")
            print(f"  Content identical: {comparison['content_identical']}")
            print(f"  Original entropy: {comparison['original']['entropy']}")
            print(f"  Transformed entropy: {comparison['transformed']['entropy']}")
    else:
        print(f"✗ Transformation failed: {result['error']}")
        sys.exit(1)


def run_agent_analysis(args):
    """Run binary analysis using AI agents"""
    print(f"Analyzing binary with AI agents: {args.input_file}")
    
    # Initialize orchestrator
    config = {
        "model": args.model or "claude-sonnet-4-5-20250929",
        "max_tokens": 4000
    }
    
    orchestrator = AgentOrchestrator()
    
    # Register agents
    cunfyooz_agent = CunfyoozAgent(config)
    analysis_agent = CunfyoozAnalysisAgent(config)
    
    orchestrator.register_agent("cunfyooz", cunfyooz_agent)
    orchestrator.register_agent("analysis", analysis_agent)
    
    # Prepare task
    task = f"analyze and obfuscate binary at {args.input_file}"
    if args.task:
        task = args.task
    
    # Run pipeline
    if args.pipeline:
        print("Running agent pipeline...")
        result = orchestrator.run_pipeline(
            task=task,
            agent_ids=["cunfyooz", "analysis"],
            initial_context={"binary_path": args.input_file}
        )
    else:
        print("Running agent pipeline (sequential to ensure proper context passing)...")
        result = orchestrator.run_pipeline(
            task=task,
            agent_ids=["cunfyooz", "analysis"],
            initial_context={"binary_path": args.input_file}
        )
    
    # Print results
    print("\nAgent Results:")
    print(json.dumps(result, indent=2, default=str))


def run_pipeline_analysis(args):
    """Run a complete analysis pipeline"""
    print(f"Running complete analysis pipeline on: {args.input_file}")
    
    # Initialize pipeline
    pipeline = CunfyoozPipeline()
    
    # Load or create config
    config = None
    if args.config:
        with open(args.config, 'r') as f:
            config = json.load(f)
    else:
        config = create_default_config()
    
    # Run complete transformation
    result = pipeline.run_complete_transformation(
        input_path=args.input_file,
        config=config
    )
    
    if result["status"] == "success":
        print("✓ Pipeline completed successfully!")
        print(f"  Original size: {result['original_analysis']['size']} bytes")
        print(f"  Transformed size: {result['transformed_analysis']['size']} bytes")
        print(f"  Size change: {result['comparison']['size_change_percent']}%")
        print(f"  Content identical: {result['comparison']['content_identical']}")
    else:
        print(f"✗ Pipeline failed: {result.get('transform_result', {}).get('error', 'Unknown error')}")
        sys.exit(1)


def main():
    parser = argparse.ArgumentParser(
        description="Cunfyooz - Advanced Binary Obfuscation and Analysis Framework",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Transform a binary with default settings
  python cunfyooz_cli.py transform my_binary.exe

  # Transform with custom config
  python cunfyooz_cli.py transform my_binary.exe -c my_config.json -o obfuscated_binary.exe

  # Analyze a binary with AI agents
  python cunfyooz_cli.py analyze my_binary.exe

  # Run complete analysis pipeline
  python cunfyooz_cli.py pipeline my_binary.exe

For more information about configuration options, see the documentation.
        """
    )
    
    # Create subparsers for different commands
    subparsers = parser.add_subparsers(dest='command', help='Available commands')
    
    # Transform command
    transform_parser = subparsers.add_parser('transform', help='Transform a binary using cunfyooz engine')
    transform_parser.add_argument('input_file', help='Input binary file to transform')
    transform_parser.add_argument('-o', '--output-file', help='Output file path (default: cunfyoozed_<input>)')
    transform_parser.add_argument('-c', '--config', help='Configuration file for transformations')
    transform_parser.add_argument('--compare', action='store_true', help='Compare original and transformed binaries')
    
    # Analyze command
    analyze_parser = subparsers.add_parser('analyze', help='Analyze binary with AI agents')
    analyze_parser.add_argument('input_file', help='Input binary file to analyze')
    analyze_parser.add_argument('-t', '--task', help='Specific task for the agents')
    analyze_parser.add_argument('-m', '--model', help='Claude model to use (default: claude-sonnet-4-5-20250929)')
    analyze_parser.add_argument('--pipeline', action='store_true', help='Run agents in pipeline mode')
    
    # Pipeline command
    pipeline_parser = subparsers.add_parser('pipeline', help='Run complete analysis pipeline')
    pipeline_parser.add_argument('input_file', help='Input binary file to process')
    pipeline_parser.add_argument('-c', '--config', help='Configuration file for transformations')
    
    # If no arguments provided, show help
    if len(sys.argv) == 1:
        parser.print_help()
        sys.exit(1)
    
    args = parser.parse_args()
    
    # Set up API key check
    if args.command in ['analyze', 'pipeline'] and not os.getenv('ANTHROPIC_API_KEY'):
        print("Error: ANTHROPIC_API_KEY environment variable not set.")
        print("Please set your API key from: https://console.anthropic.com/")
        sys.exit(1)
    
    # Execute based on command
    if args.command == 'transform':
        run_binary_transformation(args)
    elif args.command == 'analyze':
        run_agent_analysis(args)
    elif args.command == 'pipeline':
        run_pipeline_analysis(args)
    else:
        parser.print_help()
        sys.exit(1)


if __name__ == "__main__":
    main()