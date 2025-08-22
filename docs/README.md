# Documentation Index

This directory contains documentation for integrating the ESP32 Fencing Scoring Device with Home Assistant.

## Files

| File | Description |
|------|-------------|
| [home-assistant-integration.md](home-assistant-integration.md) | **Main Integration Guide** - Comprehensive documentation with MQTT setup, sensor configuration, automations, scripts, and dashboard examples |
| [yaml-troubleshooting.md](yaml-troubleshooting.md) | **Troubleshooting Guide** - Specific solutions for common YAML syntax errors, including those mentioned in issue #29 |
| [home-assistant-example-config.yaml](home-assistant-example-config.yaml) | **Example Configuration** - Copy-paste ready YAML configuration for quick setup |

## Quick Start

1. **Start here**: [home-assistant-integration.md](home-assistant-integration.md) for complete setup instructions
2. **Having issues?**: Check [yaml-troubleshooting.md](yaml-troubleshooting.md) for syntax error solutions
3. **Want examples?**: Use [home-assistant-example-config.yaml](home-assistant-example-config.yaml) as a starting point

## Key Features

- ✅ **Correct YAML syntax** - All examples follow Home Assistant requirements
- ✅ **MQTT integration** - Complete sensor and automation examples  
- ✅ **Proper templating** - Using Home Assistant's templating conventions
- ✅ **Error solutions** - Specific fixes for common syntax issues
- ✅ **Validated examples** - All YAML has been syntax validated

## Addressing Issue #29

These documents specifically address the YAML syntax errors mentioned in issue #29:

- **No more `{% if %}` blocks in YAML mappings** - Uses `choose` actions instead
- **Proper conditional logic** - Shows correct Home Assistant patterns
- **No mixed YAML/Jinja structures** - Clean separation of concerns
- **Validated syntax** - All examples are tested and working

## Contributing

Found an issue or want to improve the documentation? Please open an issue or submit a pull request!