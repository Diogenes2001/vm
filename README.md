# vm
CS246E final project

A text editor for Linux written in C++ based on the vim editor. To run:
```
cd src
make
./vm [text file] [custom theme]
```

vm supports the following features:
+ page navigation using commands
+ command, insert, and replace mode
+ saving and quitting
+ unlimited number of undoes
+ basic searching (no regexes)
+ repetition of previous commands
+ multipliers in front of commands
+ line wrapping
+ syntax highlighting
+ macros
+ custom themes (not available on all terminals)

Note that arrow keys are not supported, so the `h/j/k/l` vim commands must be used to navigate.

The full list of supported vim commands is: 

```
a b cc c[any motion] dd d[any motion] f h i j k l n o p q r s u w x yy y[any motion] A F I J N O P R S X ^ $ 0 . ; / ? % @ ^b ^d ^f ^g ^u :w :q :wq :q! :r :0 :$ :line-number
```

To toggle color mode and syntax highlighting, use the command `:color`.

To use custom themes, run the executable with a .color file as the second command line argument (not available on all terminals).

To create a .color file, enter 7 lines of space-separated RGB values for the following objects:
1. KEYWORDS
2. STRING AND NUMERIC LITERALS
3. TYPES
4. COMMENTS
5. PREPROCESSOR DIRECTIVES
6. IDENTIFIERS
7. BACKGROUND

### Default theme
![vm default theme](./vm1.png)

### Custom theme
![vm custom theme](./vm2.png)
