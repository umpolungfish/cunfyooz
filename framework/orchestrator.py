"""
Agent Orchestrator
Coordinates execution of single or multiple agents in parallel.
"""
from typing import Dict, List, Optional, Any
from concurrent.futures import ThreadPoolExecutor, as_completed
import logging

from .base_agent import BaseAgent, AgentStatus


logger = logging.getLogger(__name__)


class AgentOrchestrator:
    """
    Orchestrates agent execution in single or multi-agent (swarm) modes.

    Features:
    - Single agent execution
    - Parallel multi-agent swarms
    - Sequential pipeline execution
    - Context passing between agents
    - Result aggregation
    """

    def __init__(self, config: Optional[Dict[str, Any]] = None):
        """
        Initialize orchestrator.

        Args:
            config: Configuration dict with:
                - max_concurrent_agents: Max parallel agents (default: 5)
                - timeout: Default timeout in seconds (default: 300)
        """
        self.config = config or {}
        self.agents: Dict[str, BaseAgent] = {}
        self.max_concurrent = self.config.get("max_concurrent_agents", 5)

    def register_agent(self, agent_id: str, agent: BaseAgent) -> None:
        """
        Register an agent with the orchestrator.

        Args:
            agent_id: Unique identifier for the agent
            agent: Agent instance
        """
        self.agents[agent_id] = agent
        logger.info(f"Registered agent: {agent_id} ({agent.name})")

    def run_agent(
        self,
        agent_id: str,
        task: str,
        context: Optional[Dict[str, Any]] = None
    ) -> Dict[str, Any]:
        """
        Execute a single agent.

        Args:
            agent_id: ID of agent to run
            task: Task description
            context: Optional context data

        Returns:
            Result dictionary with status, findings, artifacts
        """
        if agent_id not in self.agents:
            raise ValueError(f"Agent not found: {agent_id}")

        agent = self.agents[agent_id]
        logger.info(f"Running agent: {agent_id}")

        try:
            agent.start()
            result = agent.run(task, context)
            agent.complete(result)
            logger.info(f"Agent {agent_id} completed successfully")
            return result
        except Exception as e:
            logger.error(f"Agent {agent_id} failed: {str(e)}")
            agent.fail(str(e))
            return {
                "status": "error",
                "error": str(e),
                "agent_id": agent_id
            }

    def run_swarm(
        self,
        task: str,
        agent_ids: Optional[List[str]] = None,
        context: Optional[Dict[str, Any]] = None
    ) -> Dict[str, Any]:
        """
        Execute multiple agents in parallel (swarm mode).

        Args:
            task: Task description for all agents
            agent_ids: List of agent IDs to run (None = all agents)
            context: Optional shared context

        Returns:
            Aggregated results dict with:
            - agents_run: Number of agents executed
            - successful: Number of successful agents
            - failed: Number of failed agents
            - results: Dict of agent_id -> result
        """
        if agent_ids is None:
            agent_ids = list(self.agents.keys())

        logger.info(f"Running swarm with {len(agent_ids)} agents")

        results = {}
        successful = 0
        failed = 0

        with ThreadPoolExecutor(max_workers=self.max_concurrent) as executor:
            # Submit all agent tasks
            future_to_agent = {
                executor.submit(self.run_agent, agent_id, task, context): agent_id
                for agent_id in agent_ids
            }

            # Collect results as they complete
            for future in as_completed(future_to_agent):
                agent_id = future_to_agent[future]
                try:
                    result = future.result()
                    results[agent_id] = result

                    if result.get("status") == "success":
                        successful += 1

                        # If the result contains context, merge it with the global context
                        if "context" in result:
                            if context is None:
                                context = {}
                            context.update(result["context"])
                    else:
                        failed += 1

                except Exception as e:
                    logger.error(f"Agent {agent_id} execution error: {str(e)}")
                    results[agent_id] = {
                        "status": "error",
                        "error": str(e)
                    }
                    failed += 1

        logger.info(f"Swarm complete: {successful} successful, {failed} failed")

        return {
            "agents_run": len(agent_ids),
            "successful": successful,
            "failed": failed,
            "results": results
        }

    def run_pipeline(
        self,
        task: str,
        agent_ids: List[str],
        initial_context: Optional[Dict[str, Any]] = None
    ) -> Dict[str, Any]:
        """
        Execute agents sequentially, passing context between stages.

        Each agent receives the previous agent's output as context.

        Args:
            task: Base task description
            agent_ids: Ordered list of agent IDs for pipeline
            initial_context: Initial context for first agent

        Returns:
            Final result dict with full pipeline execution history
        """
        logger.info(f"Running pipeline with {len(agent_ids)} stages")

        context = initial_context or {}
        pipeline_results = []

        for i, agent_id in enumerate(agent_ids):
            stage_num = i + 1
            logger.info(f"Pipeline stage {stage_num}/{len(agent_ids)}: {agent_id}")

            # Run agent with accumulated context
            result = self.run_agent(agent_id, task, context)

            # Store stage result
            pipeline_results.append({
                "stage": stage_num,
                "agent_id": agent_id,
                "result": result
            })

            # Check for failure
            if result.get("status") != "success":
                logger.warning(f"Pipeline failed at stage {stage_num}")
                return {
                    "status": "failed",
                    "failed_at_stage": stage_num,
                    "pipeline_results": pipeline_results
                }

            # Pass output as context to next stage
            context.update({
                "previous_stage": result,
                "previous_agent": agent_id
            })

        logger.info("Pipeline completed successfully")

        return {
            "status": "success",
            "stages_completed": len(agent_ids),
            "pipeline_results": pipeline_results,
            "final_context": context
        }

    def get_agent_status(self, agent_id: str) -> AgentStatus:
        """Get current status of an agent"""
        if agent_id not in self.agents:
            raise ValueError(f"Agent not found: {agent_id}")
        return self.agents[agent_id].status

    def get_all_agents(self) -> Dict[str, Dict[str, Any]]:
        """Get info about all registered agents"""
        return {
            agent_id: {
                "name": agent.name,
                "description": agent.description,
                "capabilities": agent.capabilities,
                "status": agent.status.value
            }
            for agent_id, agent in self.agents.items()
        }
