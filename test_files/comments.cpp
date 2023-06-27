/*
Copyright 2018 Adobe
All Rights Reserved.

NOTICE: Adobe permits you to use, modify, and distribute this file in
accordance with the terms of the Adobe license agreement accompanying
it. If you have received this file from a source other than Adobe,
then your use, modification, or distribution of it requires the prior
written permission of Adobe.
*/

// AST dump with
// TERM="" clang --std=c++1z -Xclang -ast-dump -fsyntax-only -fparse-all-comments -fcomment-block-commands=hyde ./comments.cpp

/// An example struct from which these commands will hang.
/// @brief This is a sample brief.
/// @warning This is a sample warning.
/// @par header This is a sample paragraph.
/// @hyde-owner fosterbrereton
/// @see [An LLVM dev meeting chat on the comment parsing feature](https://llvm.org/devmtg/2012-11/Gribenko_CommentParsing.pdf)
struct some_struct {
    virtual ~some_struct() = delete;

    /// This is a longer description of this function that does things as well as it does.
    /// Notice how long this comment is! So impressive. &#x1F4A5;
    /// @brief A function that does a thing, and does it well.
    /// @param[in] input an input parameter
    /// @param[in,out] input_output a bidirectional parameter
    /// @param[out] output an output parameter
    /// @pre An example precondition.
    /// @post An example postcondition.
    /// @return Some additional value.
    /// @throw std::runtime_error if the function actually _can't_ do the thing. Sorry!
    /// @todo This really could use some cleanup. Although, its implementation doesn't exist...
    int some_function(int input, int& input_output, int& output);

    /// A virtual function that intends to be overridden.
    virtual void virtual_function();

    int _x{0}; ///< A trailing comment that documents `_x`.
};

/// Notice how many of the comments for this structure are inherited from its superclass.
/// @brief This is a sample brief for `some_other_struct`
struct some_other_struct : public some_struct {
    void virtual_function() override;
};

/// @brief some template function
/// @tparam T The type of thing being returned
/// @return an instance of type `T`
template <class T>
T template_function();

//------------------------------------------------------------------------------------------------------------------------------------------
