# Home Assistant YAML Troubleshooting Guide

This guide addresses common YAML syntax errors when integrating ESP32 devices with Home Assistant, particularly focusing on the issues mentioned in GitHub issue #29.

## Common YAML Syntax Errors and Solutions

### 1. Using Unsupported Jinja Block Syntax

**❌ WRONG - Don't use Jinja blocks in YAML mappings:**
```yaml
automation:
  - alias: "Bad Example"
    trigger:
      platform: state
      entity_id: sensor.piste_status
    action:
      {% if states('sensor.piste_status') == 'H' %}
      service: notify.mobile_app
      data:
        message: "Match started"
      {% endif %}
```

**✅ CORRECT - Use Home Assistant's choose action:**
```yaml
automation:
  - alias: "Good Example"
    trigger:
      platform: state
      entity_id: sensor.piste_status
    action:
      choose:
        - conditions:
            - condition: state
              entity_id: sensor.piste_status
              state: "H"
          sequence:
            - service: notify.mobile_app
              data:
                message: "Match started"
```

### 2. Mixing YAML Structure with Jinja Logic

**❌ WRONG - Don't mix control structures:**
```yaml
script:
  notify_all_pistes:
    sequence:
      {% for piste_id in range(1, 9) %}
      - service: notify.persistent_notification
        data:
          message: "Piste {{ piste_id }} status"
      {% endfor %}
```

**✅ CORRECT - Use repeat action:**
```yaml
script:
  notify_all_pistes:
    sequence:
      - repeat:
          count: 8
          sequence:
            - service: notify.persistent_notification
              data:
                message: "Piste {{ repeat.index }} status"
```

### 3. Improper Template Usage in Conditions

**❌ WRONG - Complex Jinja blocks in condition mappings:**
```yaml
automation:
  trigger:
    platform: state
    entity_id: sensor.score
  condition:
    {% if states('sensor.left_score') | int > states('sensor.right_score') | int %}
    - condition: state
      entity_id: sensor.match_state
      state: "active"
    {% endif %}
```

**✅ CORRECT - Use template condition properly:**
```yaml
automation:
  trigger:
    platform: state
    entity_id: sensor.score
  condition:
    - condition: template
      value_template: >
        {{ states('sensor.left_score') | int > states('sensor.right_score') | int
           and states('sensor.match_state') == 'active' }}
```

### 4. Incorrect Loop Structure in Scripts

**❌ WRONG - Jinja loops in YAML structure:**
```yaml
script:
  reset_all_pistes:
    sequence:
      {% for piste in ['001', '002', '003'] %}
      - service: mqtt.publish
        data:
          topic: "MQTT_Cyrano/Piste_{{ piste }}/FromSoftware"
          payload: '{"command": "reset"}'
      {% endfor %}
```

**✅ CORRECT - Use repeat with template:**
```yaml
script:
  reset_all_pistes:
    variables:
      pistes: ['001', '002', '003']
    sequence:
      - repeat:
          for_each: "{{ pistes }}"
          sequence:
            - service: mqtt.publish
              data:
                topic: "MQTT_Cyrano/Piste_{{ repeat.item }}/FromSoftware"
                payload: '{"command": "reset"}'
```

### 5. Improper Use of Templates in Value Assignments

**❌ WRONG - Jinja blocks where values expected:**
```yaml
sensor:
  - platform: mqtt
    name: "Dynamic Sensor"
    state_topic: >
      {% if states('input_select.piste') == 'piste_1' %}
        MQTT_Cyrano/Piste_001/FromDevice
      {% else %}
        MQTT_Cyrano/Piste_002/FromDevice
      {% endif %}
```

**✅ CORRECT - Use template sensor or proper templating:**
```yaml
sensor:
  - platform: template
    sensors:
      dynamic_piste_sensor:
        friendly_name: "Dynamic Piste Sensor"
        value_template: >
          {% if states('input_select.piste') == 'piste_1' %}
            {{ states('sensor.piste_1_status') }}
          {% else %}
            {{ states('sensor.piste_2_status') }}
          {% endif %}
```

## Best Practices for Home Assistant YAML

### 1. Use Proper Home Assistant Actions

Instead of Jinja control structures, use Home Assistant's built-in actions:

- **choose**: For conditional logic
- **repeat**: For loops  
- **if**: For simple conditional service calls (newer HA versions)
- **variables**: For defining reusable values

### 2. Template Placement Rules

- **value_template**: Use for extracting/transforming values
- **condition template**: Use for complex logical conditions
- **service_template**: Use for dynamic service calls
- **data_template**: Use for dynamic data payloads

### 3. Validation Steps

1. **Always validate YAML syntax** using online validators
2. **Test templates** in Developer Tools → Template
3. **Check configuration** before restarting Home Assistant
4. **Use the Template Editor** to develop complex templates

### 4. Debugging Templates

Use the Template tool in Home Assistant Developer Tools:

```jinja2
{% set piste_state = states('sensor.piste_1_status') %}
{% set left_score = states('sensor.piste_1_left_score') | int %}
{% set right_score = states('sensor.piste_1_right_score') | int %}

Current state: {{ piste_state }}
Scores: {{ left_score }} - {{ right_score }}
Winner: {% if left_score > right_score %}Left{% elif right_score > left_score %}Right{% else %}Tie{% endif %}
```

## ESP32 Scoring Device Specific Examples

### Correct MQTT Sensor Configuration

```yaml
sensor:
  - platform: mqtt
    name: "Piste Status"
    state_topic: "MQTT_Cyrano/Piste_001/FromDevice"
    value_template: "{{ value_json.state | default('unknown') }}"
    json_attributes_topic: "MQTT_Cyrano/Piste_001/FromDevice"
    unique_id: "piste_001_status"
```

### Correct Automation for Match Events

```yaml
automation:
  - alias: "Match State Changes"
    trigger:
      - platform: state
        entity_id: sensor.piste_status
    action:
      choose:
        - conditions:
            - condition: state
              entity_id: sensor.piste_status
              state: "H"  # Halted - match in progress
          sequence:
            - service: light.turn_on
              target:
                entity_id: light.piste_indicator
              data:
                color_name: red
        - conditions:
            - condition: state
              entity_id: sensor.piste_status
              state: "W"  # Waiting
          sequence:
            - service: light.turn_on
              target:
                entity_id: light.piste_indicator
              data:
                color_name: green
        default:
          - service: light.turn_off
            target:
              entity_id: light.piste_indicator
```

### Correct Script for Device Control

```yaml
script:
  control_piste:
    alias: "Control Piste"
    fields:
      piste_number:
        description: "Piste number (001-999)"
        example: "001"
      command:
        description: "Command to send"
        example: "HELLO"
    sequence:
      - service: mqtt.publish
        data:
          topic: "MQTT_Cyrano/Piste_{{ piste_number }}/FromSoftware"
          payload: >
            {
              "protocol": "EFP1",
              "command": "{{ command }}",
              "piste_id": "{{ piste_number }}"
            }
```

## Tools for Validation

1. **YAML Validator**: https://yamlchecker.com/
2. **Home Assistant Configuration Check**: `ha core check`
3. **Template Editor**: Developer Tools → Template in Home Assistant
4. **Configuration Validation**: Check Configuration in Home Assistant

## Quick Reference: Do's and Don'ts

### ✅ DO:
- Use `choose` for conditional logic in automations
- Use `repeat` for loops in scripts
- Use `value_template` for data extraction
- Use `condition: template` for complex conditions
- Test all templates before deployment
- Validate YAML syntax

### ❌ DON'T:
- Use `{% if %}` blocks in YAML mappings
- Mix Jinja control structures with YAML structure
- Use `{% for %}` loops in YAML sequences
- Put Jinja blocks where YAML values are expected
- Skip validation of YAML syntax
- Deploy untested templates

## Getting Help

If you encounter issues:

1. Check this troubleshooting guide
2. Validate your YAML syntax
3. Test templates in the Template Editor
4. Check Home Assistant logs for errors
5. Consult the [main integration guide](home-assistant-integration.md)
6. Open an issue with specific error messages and configuration

Remember: Home Assistant has specific requirements for YAML structure and template usage. Always follow the platform's conventions rather than trying to use generic Jinja templating approaches.