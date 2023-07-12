---
layout: function
title: overloaded
hyde:
  owner: __MISSING__
  brief: __INLINED__
  tags:
    - function
  inline:
    brief: _multiple descriptions_
  defined_in_file: functions.cpp
  overloads:
    auto overloaded(int) -> float:
      arguments:
        - description: __OPTIONAL__
          name: first
          type: int
      description: __INLINED__
      inline:
        arguments:
          first:
            description: the first input parameter
        description:
          - an example unary overloaded function
        return: "`first`"
      return: __OPTIONAL__
      signature_with_names: auto overloaded(int first) -> float
    auto overloaded(int, int) -> double:
      arguments:
        - description: __OPTIONAL__
          name: first
          type: int
        - description: __OPTIONAL__
          name: second
          type: int
      description: __INLINED__
      inline:
        arguments:
          first:
            description: the first input parameter
          second:
            description: the second input parameter
        description:
          - an example binary overloaded function
        return: the product of `first` and `second`
      return: __OPTIONAL__
      signature_with_names: auto overloaded(int first, int second) -> double
    auto overloaded(int, int, int) -> float:
      arguments:
        - description: __OPTIONAL__
          name: first
          type: int
        - description: __OPTIONAL__
          name: second
          type: int
        - description: __OPTIONAL__
          name: third
          type: int
      description: __INLINED__
      inline:
        arguments:
          first:
            description: the first input parameter
          second:
            description: the second input parameter
          third:
            description: the third input parameter
        description:
          - an example tertiary overloaded function
        return: the product of `first`, `second`, and `third`
      return: __OPTIONAL__
      signature_with_names: auto overloaded(int first, int second, int third) -> float
---
