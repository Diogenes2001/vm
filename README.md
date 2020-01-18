# vm
CS246E final project (executable only)

A text editor for Linux based on the vm editor. It supports the following:
+ syntax highlighting
+ macros
+ custom themes
+ 55 commands: a b cc c[any motion] dd d[any motion] f h i j k l n o p q r s u w x yy y[any motion] A F I J N O P R S X ^ $ 0 . ; / ? % @ ^b ^d ^f ^g ^u :w :q :wq :q! :r :0 :$ :line-number

To use custom themes, run the executable with a .color file as a command line argument (does not work on PuTTY).

To create a .color file, enter 7 lines of space-separated RGB values in the following order:
1. KEYWORDS
2. STRING AND NUMERIC LITERALS
3. TYPES
4. COMMENTS
5. PREPROCESSOR DIRECTIVES
6. IDENTIFIERS
7. BACKGROUND


Due to University of Waterloo policies, I am prohibited from publishing the source code for this project. However, I have included the executable and some screenshots:
