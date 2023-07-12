---
layout: method
title: some_function
hyde:
  owner: __INLINED__
  brief: __INLINED__
  tags:
    - method
  inline:
    brief: A function that does a thing, and does it well.
    owner: fosterbrereton
  defined_in_file: comments.cpp
  overloads:
    int some_function(int, int &, int &):
      arguments:
        - description: __OPTIONAL__
          name: input
          type: int
        - description: __OPTIONAL__
          name: input_output
          type: int &
        - description: __OPTIONAL__
          name: output
          type: int &
      description: __INLINED__
      inline:
        arguments:
          input:
            description: an input parameter
            direction: in
          input_output:
            description: a bidirectional parameter
            direction: inout
          output:
            description: an output parameter
            direction: out
        brief: A function that does a thing, and does it well.
        description:
          - This is a longer description of this function that does things as well as it does. Notice how long this comment is! So impressive. 💥
        post: An example postcondition.
        pre: An example precondition.
        return: Some additional value.
        throw: "`std::runtime_error` if the function actually _can't_ do the thing. Sorry!"
        todo: This really could use some cleanup. Although, its implementation doesn't exist...
        warning: This function may be very expensive to run. Do not call it inside a loop.
      return: __OPTIONAL__
      signature_with_names: int some_function(int input, int & input_output, int & output)
---
