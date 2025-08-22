# Home Assistant Integration Guide

This guide provides comprehensive examples for integrating the ESP32 Fencing Scoring Device with Home Assistant using MQTT. All YAML examples follow Home Assistant's syntax requirements and best practices.

## Table of Contents

1. [MQTT Configuration](#mqtt-configuration)
2. [Sensor Configuration](#sensor-configuration)
3. [Automation Examples](#automation-examples)
4. [Script Examples](#script-examples)
5. [Dashboard Examples](#dashboard-examples)
6. [Troubleshooting](#troubleshooting)

## MQTT Configuration

### Device MQTT Topics

The ESP32 scoring device publishes to these MQTT topics:
- `MQTT_Cyrano/Piste_XXX/FromDevice` - Device status and scoring data
- `MQTT_Cyrano/Piste_XXX/Connection` - Device connection status
- `MQTT_Cyrano/Piste_XXX/FromSoftware` - Commands to device (subscribed)

Where `XXX` is the piste number (e.g., 001, 002, etc.).

### Basic MQTT Broker Configuration

Add to your `configuration.yaml`:

```yaml
mqtt:
  broker: 192.168.1.100  # Your MQTT broker IP
  port: 1883
  username: !secret mqtt_username  # Optional
  password: !secret mqtt_password  # Optional
```

## Sensor Configuration

### Device Connection Status

```yaml
mqtt:
  binary_sensor:
    - name: "Piste 1 Connection"
      state_topic: "MQTT_Cyrano/Piste_001/Connection"
      payload_on: "online"
      payload_off: "offline"
      device_class: connectivity
      unique_id: "piste_001_connection"

sensor:
  - platform: mqtt
    name: "Piste 1 Status"
    state_topic: "MQTT_Cyrano/Piste_001/FromDevice"
    value_template: "{{ value_json.state | default('unknown') }}"
    json_attributes_topic: "MQTT_Cyrano/Piste_001/FromDevice"
    unique_id: "piste_001_status"
```

### Scoring Data Sensors

```yaml
sensor:
  - platform: mqtt
    name: "Piste 1 Left Score"
    state_topic: "MQTT_Cyrano/Piste_001/FromDevice"
    value_template: "{{ value_json.left_score | default(0) | int }}"
    unit_of_measurement: "points"
    unique_id: "piste_001_left_score"

  - platform: mqtt
    name: "Piste 1 Right Score"
    state_topic: "MQTT_Cyrano/Piste_001/FromDevice"
    value_template: "{{ value_json.right_score | default(0) | int }}"
    unit_of_measurement: "points"
    unique_id: "piste_001_right_score"

  - platform: mqtt
    name: "Piste 1 Timer"
    state_topic: "MQTT_Cyrano/Piste_001/FromDevice"
    value_template: "{{ value_json.stopwatch | default('00:00') }}"
    unique_id: "piste_001_timer"

  - platform: mqtt
    name: "Piste 1 Left Fencer"
    state_topic: "MQTT_Cyrano/Piste_001/FromDevice"
    value_template: "{{ value_json.left_fencer_name | default('Unknown') }}"
    unique_id: "piste_001_left_fencer"

  - platform: mqtt
    name: "Piste 1 Right Fencer"
    state_topic: "MQTT_Cyrano/Piste_001/FromDevice"
    value_template: "{{ value_json.right_fencer_name | default('Unknown') }}"
    unique_id: "piste_001_right_fencer"
```

### Light Indicators

```yaml
binary_sensor:
  - platform: mqtt
    name: "Piste 1 Red Light"
    state_topic: "MQTT_Cyrano/Piste_001/FromDevice"
    value_template: "{{ value_json.left_light == '1' }}"
    payload_on: true
    payload_off: false
    unique_id: "piste_001_red_light"

  - platform: mqtt
    name: "Piste 1 Green Light"
    state_topic: "MQTT_Cyrano/Piste_001/FromDevice"
    value_template: "{{ value_json.right_light == '1' }}"
    payload_on: true
    payload_off: false
    unique_id: "piste_001_green_light"

  - platform: mqtt
    name: "Piste 1 White Light Left"
    state_topic: "MQTT_Cyrano/Piste_001/FromDevice"
    value_template: "{{ value_json.left_white_light == '1' }}"
    payload_on: true
    payload_off: false
    unique_id: "piste_001_white_light_left"

  - platform: mqtt
    name: "Piste 1 White Light Right"
    state_topic: "MQTT_Cyrano/Piste_001/FromDevice"
    value_template: "{{ value_json.right_white_light == '1' }}"
    payload_on: true
    payload_off: false
    unique_id: "piste_001_white_light_right"
```

## Automation Examples

### Match Start Notification

```yaml
automation:
  - alias: "Piste 1 Match Started"
    description: "Notify when a match starts on Piste 1"
    trigger:
      - platform: state
        entity_id: sensor.piste_1_status
        from: "W"  # Waiting
        to: "H"    # Halted (match in progress)
    action:
      - service: notify.persistent_notification
        data:
          title: "Fencing Match Started"
          message: >
            Match started on Piste 1:
            {{ states('sensor.piste_1_left_fencer') }} vs {{ states('sensor.piste_1_right_fencer') }}

  - alias: "Piste 1 Touch Scored"
    description: "Notify when a touch is scored"
    trigger:
      - platform: state
        entity_id: sensor.piste_1_left_score
      - platform: state
        entity_id: sensor.piste_1_right_score
    condition:
      - condition: template
        value_template: >
          {{ trigger.to_state.state | int > trigger.from_state.state | int }}
    action:
      - service: notify.mobile_app_your_phone
        data:
          title: "Touch Scored!"
          message: >
            {% if trigger.entity_id == 'sensor.piste_1_left_score' %}
              {{ states('sensor.piste_1_left_fencer') }} scored! 
              Score: {{ states('sensor.piste_1_left_score') }} - {{ states('sensor.piste_1_right_score') }}
            {% else %}
              {{ states('sensor.piste_1_right_fencer') }} scored! 
              Score: {{ states('sensor.piste_1_left_score') }} - {{ states('sensor.piste_1_right_score') }}
            {% endif %}
```

### Light Flash Automation

```yaml
automation:
  - alias: "Piste 1 Light Flash Alert"
    description: "Flash room lights when fencing lights activate"
    trigger:
      - platform: state
        entity_id: 
          - binary_sensor.piste_1_red_light
          - binary_sensor.piste_1_green_light
        to: "on"
    action:
      - service: light.turn_on
        target:
          entity_id: light.training_room_lights
        data:
          flash: short
          color_name: >
            {% if trigger.entity_id == 'binary_sensor.piste_1_red_light' %}
              red
            {% else %}
              green
            {% endif %}
```

## Script Examples

### Reset Piste Script

```yaml
script:
  reset_piste_1:
    alias: "Reset Piste 1"
    description: "Send reset command to Piste 1"
    sequence:
      - service: mqtt.publish
        data:
          topic: "MQTT_Cyrano/Piste_001/FromSoftware"
          payload: >
            {
              "protocol": "EFP1",
              "command": "DISP",
              "piste_id": "001",
              "left_score": "0",
              "right_score": "0",
              "state": "W"
            }

  start_bout_piste_1:
    alias: "Start Bout on Piste 1"
    description: "Start a bout on Piste 1"
    sequence:
      - service: mqtt.publish
        data:
          topic: "MQTT_Cyrano/Piste_001/FromSoftware" 
          payload: >
            {
              "protocol": "EFP1",
              "command": "DISP",
              "piste_id": "001",
              "state": "H"
            }

  emergency_stop_all_pistes:
    alias: "Emergency Stop All Pistes"
    description: "Emergency stop for all pistes"
    sequence:
      - repeat:
          count: 8  # For 8 pistes
          sequence:
            - service: mqtt.publish
              data:
                topic: "MQTT_Cyrano/Piste_{{ '%03d' | format(repeat.index) }}/FromSoftware"
                payload: >
                  {
                    "protocol": "EFP1", 
                    "command": "DISP",
                    "piste_id": "{{ '%03d' | format(repeat.index) }}",
                    "state": "E"
                  }
```

### Conditional Script with Choose

```yaml
script:
  piste_action_based_on_state:
    alias: "Piste Action Based on State"
    description: "Perform different actions based on piste state"
    fields:
      piste_number:
        description: "Piste number (001-999)"
        example: "001"
    sequence:
      - choose:
          - conditions:
              - condition: state
                entity_id: "sensor.piste_{{ piste_number }}_status"
                state: "W"  # Waiting
            sequence:
              - service: mqtt.publish
                data:
                  topic: "MQTT_Cyrano/Piste_{{ piste_number }}/FromSoftware"
                  payload: >
                    {
                      "protocol": "EFP1",
                      "command": "HELLO",
                      "piste_id": "{{ piste_number }}"
                    }
          - conditions:
              - condition: state
                entity_id: "sensor.piste_{{ piste_number }}_status" 
                state: "H"  # Halted
            sequence:
              - service: notify.persistent_notification
                data:
                  title: "Piste Status"
                  message: "Piste {{ piste_number }} is currently in use"
        default:
          - service: notify.persistent_notification
            data:
              title: "Piste Status"
              message: "Piste {{ piste_number }} state: {{ states('sensor.piste_' + piste_number + '_status') }}"
```

## Dashboard Examples

### Piste Status Card

```yaml
type: entities
title: "Piste 1 Status"
entities:
  - entity: binary_sensor.piste_1_connection
    name: "Connection"
  - entity: sensor.piste_1_status
    name: "State"
  - entity: sensor.piste_1_left_fencer
    name: "Left Fencer"
  - entity: sensor.piste_1_right_fencer
    name: "Right Fencer"
  - entity: sensor.piste_1_left_score
    name: "Left Score"
  - entity: sensor.piste_1_right_score
    name: "Right Score"
  - entity: sensor.piste_1_timer
    name: "Timer"
```

### Light Status Grid

```yaml
type: grid
columns: 2
cards:
  - type: entity
    entity: binary_sensor.piste_1_red_light
    name: "Red Light"
    icon: mdi:circle
    state_color: true
  - type: entity
    entity: binary_sensor.piste_1_green_light
    name: "Green Light" 
    icon: mdi:circle
    state_color: true
  - type: entity
    entity: binary_sensor.piste_1_white_light_left
    name: "White Left"
    icon: mdi:circle-outline
  - type: entity
    entity: binary_sensor.piste_1_white_light_right
    name: "White Right"
    icon: mdi:circle-outline
```

## Troubleshooting

### Common YAML Syntax Issues

**❌ Incorrect - Using unsupported Jinja blocks:**
```yaml
# DON'T DO THIS - {% if %} blocks are not supported in YAML mappings
automation:
  trigger:
    platform: state
    entity_id: sensor.piste_1_status
  action:
    {% if states('sensor.piste_1_status') == 'H' %}
    service: notify.persistent_notification
    {% endif %}
```

**✅ Correct - Using choose action:**
```yaml
automation:
  trigger:
    platform: state
    entity_id: sensor.piste_1_status
  action:
    choose:
      - conditions:
          - condition: state
            entity_id: sensor.piste_1_status
            state: "H"
        sequence:
          - service: notify.persistent_notification
            data:
              message: "Piste is halted"
```

**❌ Incorrect - Mixed YAML and Jinja logic:**
```yaml
# DON'T DO THIS - Mixing control structures
script:
  my_script:
    sequence:
      {% for piste in range(1, 9) %}
      - service: mqtt.publish
        data:
          topic: "MQTT_Cyrano/Piste_{{ '%03d' | format(piste) }}/FromSoftware"
      {% endfor %}
```

**✅ Correct - Using repeat:**
```yaml
script:
  my_script:
    sequence:
      - repeat:
          count: 8
          sequence:
            - service: mqtt.publish
              data:
                topic: "MQTT_Cyrano/Piste_{{ '%03d' | format(repeat.index) }}/FromSoftware"
```

### Template Testing

Use Home Assistant's Template tool (Developer Tools → Template) to test templates:

```jinja2
{% set piste_state = states('sensor.piste_1_status') %}
{% if piste_state == 'H' %}
  Match in progress
{% elif piste_state == 'W' %}
  Waiting for match
{% else %}
  State: {{ piste_state }}
{% endif %}
```

### MQTT Topic Testing

Test MQTT topics using the MQTT integration's publish service:

```yaml
service: mqtt.publish
data:
  topic: "MQTT_Cyrano/Piste_001/FromSoftware"
  payload: '{"protocol":"EFP1","command":"HELLO","piste_id":"001"}'
```

### Validation Checklist

- ✅ Use `value_template` for extracting values from JSON
- ✅ Use `choose` action for conditional logic in automations/scripts  
- ✅ Use `repeat` for loops in scripts
- ✅ Test templates in Developer Tools before using
- ✅ Use `unique_id` for all sensors
- ✅ Validate YAML syntax with a YAML validator
- ❌ Don't use `{% if %}` blocks in YAML mappings
- ❌ Don't mix Jinja control structures with YAML
- ❌ Don't forget quotes around string values in conditions

## Additional Resources

- [YAML Troubleshooting Guide](yaml-troubleshooting.md) - Detailed solutions for common YAML syntax issues
- [Example Configuration File](home-assistant-example-config.yaml) - Copy-paste ready configuration
- [Home Assistant MQTT Integration](https://www.home-assistant.io/integrations/mqtt/)
- [Home Assistant Templating](https://www.home-assistant.io/docs/configuration/templating/)
- [YAML Syntax Checker](https://yamlchecker.com/)
- [ESP32 Scoring Device Wiki](https://github.com/pietwauters/esp32-scoring-device/wiki)

## Contributing

Found an issue with these examples? Please open an issue or submit a pull request to help improve this integration guide.