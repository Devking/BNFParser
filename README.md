BNF (Backus-Naur Form) Math Expressions Parser
==============================================

Overview
--------

This repository tracks my progress in developing a BNF recursive-descent parser to evaluate mathematical expressions.

This project stems from an assignment that I had to program for a graduate Programming Languages course. The main challenge of that assignment was to pick up a new language (Ruby) and create a recursive descent parser in that language.

Prior to this assignment, I had been used to dealing with mathematical expressions in C++ by using push-down automata to convert expressions from infix notation to postfix notation before evaluating. The primary goal of this assignment was to expand my horizons with parsing (and pick up new languages along the way). This is also an attempt to add more to my GitHub and practice working with Markdown documentation for my projects.

Scripts Included/In Progress
----------------------------

| Script Name       | Script Language | Description                                     |
| ----------------- | --------------- | ----------------------------------------------- |
| assignedparser.rb | Ruby            | The original script written for the course.     |
| fullparser.rb     | Ruby            | A modified parser to make more practical sense. |
| pushdown.cpp      | C++             | An approach at parsing using pushdown automata. |

### Script Breakdowns

#### assignedparser.rb

This uses a BNF-inspired approach to parse and evaluate mathematical expressions with proper order of operations. However, this is the original version assigned from my course, with peculiar objectives that needed to be fulfilled.

Still working on documenting sample input/output cases.

#### fullparser.rb

This uses a BNF-inspired approach to parse and evaluate mathematical expressions with proper order of operations. This is a modification of the **assignedparser.rb** script in order to be more sensible in practical use.

Still working on documenting  sample input/output cases.

#### pushdown.cpp

This uses an old-school pushdown automata to convert the mathematical expressions entered in infix notation to postfix notation, so that they can be evaluated non-ambiguously. This code stems from my prior experience in C++ and working closely with the Data Structures and Algorithms course (which I have TA'ed for, for two years now), which covers this topic.

Still working on documenting  sample input/output cases.

BNF Background
--------------

Still working on writing this section...

You can read more about Backus-Naur Form [here](http://en.wikipedia.org/wiki/Backus–Naur_Form).

You can read more about recursive descent parsing [here](http://en.wikipedia.org/wiki/Recursive_descent_parser).

You can read more about postfix notation [here](http://en.wikipedia.org/wiki/Reverse_Polish_notation).