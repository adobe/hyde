---
layout: method
title: operator-=
hyde:
  owner: __MISSING__
  brief: __INLINED__
  tags:
    - method
  inline:
    brief:
      - Subtraction-assignment operator.
  defined_in_file: point.cpp
  overloads:
    constexpr point<T> & operator-=(const point<T> &):
      arguments:
        - description: __OPTIONAL__
          name: a
          type: const point<T> &
      description: __INLINED__
      inline:
        arguments:
          a:
            description: The point to subtract from this point
        description:
          - Subtraction-assignment operator.
        return: A reference to `this`.
      return: __OPTIONAL__
      signature_with_names: constexpr point<T> & operator-=(const point<T> & a)
---
