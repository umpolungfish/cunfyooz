# Changelog

All notable changes to the cunfyooz metamorphic engine will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased] - 2024-10-02

### Added
- JSON configuration parser implementation for customizable transformation parameters
- Support for loading transformation settings from external `config.json` files
- Enhanced configuration system with per-transformer intensity controls and enable/disable options
- Robust error handling for malformed configuration files
- Default configuration fallback when JSON file is missing or invalid

### Changed
- Updated documentation to reflect JSON configuration parser implementation
- Improved configuration loading mechanism in main execution flow
- Enhanced transformation pipeline to utilize JSON-driven settings
- Refined transformation probabilities based on configuration parameters

### Fixed
- Configuration-related compilation warnings
- Memory management issues in configuration parsing
- Type safety improvements in configuration value handling

## [1.0.0] - 2024-09-30

### Added
- Initial release of cunfyooz metamorphic engine
- Support for PE binary parsing and transformation
- Implementation of multiple metamorphic transformation techniques:
  - NOP insertion
  - Instruction substitution
  - Register shuffling
  - Enhanced NOP insertion
  - Control flow obfuscation
  - Stack frame obfuscation
  - Instruction reordering
  - Anti-analysis techniques
  - Virtualization engine
- Integration with Capstone disassembly engine
- Integration with Keystone assembly engine
- Comprehensive documentation and usage guides
- Support for x86-64 architecture with Intel syntax
- Modular architecture with separate components for parsing, transformation, and assembly
- Detailed error handling and validation throughout the transformation pipeline