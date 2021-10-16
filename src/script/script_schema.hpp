#include <string>

#pragma once

namespace script
{
// TODO: Add definitions for methods
const std::string schema = R"(
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "required": [
    "flow",
    "messages",
    "dns",
    "port",
    "timeout"
  ],
  "additionalProperties": false,
  "properties": {
    "dns": {
      "type": "string"
    },
    "port": {
      "type": "string"
    },
    "secure": {
      "type": "boolean"
    },
    "timeout": {
      "type": "integer"
    },
    "variables": {
      "type": "object",
      "minProperties": 1,
        "additionalProperties": {
          "oneOf": [
            {"type": "string"},
            {"type": "integer"}
          ]
      }
    },
    "ranges": {
      "type": "object",
      "additionalProperties": {
        "type": "object",
        "additionalProperties": false,
        "required": [
          "min",
          "max"
        ],
        "properties": {
          "min": {
            "type": "integer"
          },
          "max": {
            "type": "integer"
          }
        }
      }
    },
    "flow": {
      "type": "array",
      "minItems": 1,
      "items": {
        "type": "string"
      }
    },
    "messages": {
      "type": "object",
      "additionalProperties": {
        "type": "object",
        "required": [
          "url",
          "body",
          "method",
          "response"
        ],
        "additionalProperties": false,
        "properties": {
          "url": {
            "type": "string"
          },
          "body": {
            "type": "object"
          },
          "headers":{
            "type": "object",
            "minProperties": 1,
            "additionalProperties": {
              "type": "string"
            }
          },
          "method": {
            "type": "string"
          },
          "response": {
            "type": "object",
            "required": ["code"],
            "additionalProperties": false,
            "properties": {
              "code": {
                "type": "integer"
              }
            }
          },
          "save_from_answer": {
            "type": "object",
            "required": ["name", "path", "value_type"],
            "additionalProperties": false,
            "properties": {
              "name": {
                "type": "string"
              },
              "path": {
                "type": "string"
              },
              "value_type": {
                "type": "string",
                "enum": ["string", "int", "object"]
              }
            }
          },
          "add_from_saved_to_body": {
            "type": "object",
            "required": ["name", "path", "value_type"],
            "additionalProperties": false,
            "properties": {
              "name": {
                "type": "string"
              },
              "path": {
                "type": "string"
              },
              "value_type": {
                "type": "string",
                "enum": ["string", "int", "object"]
              }
            }
          }
        }
      }
    }
  }
}
)";

}  // namespace script
