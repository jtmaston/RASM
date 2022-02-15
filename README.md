# RASM
A Pseudo-Programming language designed to automate robots running my [ yet to be named ] framework

# General description
RASM ( very loosely ) follows the model of G-Code, following a syntax that can generally be summarized as [ OPCODE ARG1 ARG2 ARG3 ARG 4 ]. The name comes from a mash-up between Robot and Assembly. This is not designed to be any standard, nor replay any other standard, but rather as a quick and easy research project.

# OPCODES
 This section summarizes the opcodes used, and how to use them. As more are implemented, this list will expand. So far, the main target is the DOFBot platform, the code will therefore be prioritizing instructions for a 5-DOF articulated arm.
 Parameters marked with [ ] are optional.
## List of opcodes:
    * ANG param [ time ]
        - Opcode 0, accepts 2 parameters
        - Sets single servo to specified angle
    * ANGS `param1 [ param2 ] [ param3 ] [ param4 ] [ param5 ] 
    [ param6 ] [ time ]`
        - Opcode 1, accepts 1 to 6 parameters for angles and 1 for time
        - Sets a number of servos to the specified angles
        - Note: setting an angle to "null" tells the robot it should
        not change its value
    * DEL param
        - Delays for given time ( in ms )
    * OFS param1 param2
        - Defines an param2 as an offset for the servo with param1 
        as index
    * NME param
        - Sets a name for the program
    * SPD param
        - Defines a "global" speed to be used when the time
        parameter is left unfilled for the angles
    * GHME
        - Go home, returns robot to the home position
    * SHME [ param1 ], [ param2 ], [ param3 ], ... [ param6 ]
        - Set the robot's home to current position
        - Alternatively, if *ALL* of the params are filled in with
        valid values, home is set to them.
    * INC param1 param2
        - Increment the value of a servo by param2 degrees
        - Param2 may also be negative, resulting in a decrement
    * DEC param1 param2
        - Explicit decrement of the value of a servo
    * RPP param1, param2, param3
        - Rapid positioning to x, y, z in space, calculated by IK.
    * IPP
        - Interpolated positioning to x, y, z
    * END
        - Marks end of a program
    * GTO
        - GOTO instruction number, useful for loops
    * IF param1 CONDITION param2 GOTO param3
        - CONDITION is any of LE ( less or equal ), L ( less ), GE 
        ( greater or equal ), G ( greater ), EQ ( equal )
        - INSTRUCTION should be a reachable instruction number in the program
        - GOTO *must* be present
    * IFN param1 CONDITION param2 INSTRUCTION
        - Same as IF, however truth condition is inverted
    * ABR
        - Abort, cuts power to the servos

# Variables
I plan to also have support for variables, though this may be a bit more complex
## Variable Opcodes
    * $ param1 param2
        - declare a variable of name param1 with numeric value param2
        - NOTE: please note the space between $ and the variable name
    
    * #param1
        - use plain numeric value, without explicitly assigning a variable
        - note THE LACK of space between # and the value

    * @ param1 param2
        - declare variable of the name param1 with string value param2
        - NOTE: strings are declared read-only
    * PRT param1
        - print param1 to the debug console
    * ADD param1 param2 param3
        - puts param2 + param3 into param1
    * SUB
        - puts param2 - param3 into param1
    * DIV
        - puts param2 / param3 into param1, truncating decimals
    * FDIV
        - puts param2 / param3 into param1, keeping decimal precision
    * SQRT prarm1, param2
        - puts sqrt( param2 ) into param2
    * TRNC param1
        - discards decimals of param1

# Implementation progress
## Main opcodes

| OPCODE | Assembler support |   Interpreter support  |
|--------|-------------------|------------------------|
|ANG     |        [ x ]      |          [ x ]         |
|ANGS    |        [ x ]      |          [ x ]         |
|DEL     |        [ x ]      |          [ x ]         |
|OFS     |        [ x ]      |          [ ]           |
|NME     |        [ x ]      |          [ x ]         |
|SPD     |        [ x ]      |          [ x ]         |
|GHME    |        [ x ]      |          [ x ]         |
|SHME    |        [ x ]      |          [ ]           |
|INC     |        [ x ]      |          [ x ]         |
|DEC     |        [ x ]      |          [ x ]         |
|RPP     |        [ ]        |          [ ]           |
|IPP     |        [ ]        |          [ ]           |
|END     |        [ x ]      |          [ ]           |
|GOTO    |        [ x ]      |          [ x ]         |
|IF      |        [ ]        |          [ ]           |
|IFN     |        [ ]        |          [ ]           |
|ABR     |        [ ]        |          [ ]           |

## Logic

| OPCODE | Assembler support | Interpreter support |
|--------|-------------------|---------------------|
|LE      |        [ ]        |         [ ]         |
|L       |        [ ]        |         [ ]         |
|GE      |        [ ]        |         [ ]         |
|G       |        [ ]        |         [ ]         |
|EQ      |        [ ]        |         [ ]         |

## Variables
| OPCODE | Assembler support | Interpreter support |
|--------|-------------------|---------------------|
| #      |        [ x ]        |         [ x ]         |
| @      |        [ ]          |         [ ]           |
| PRT    |        [ x ]        |         [ x ]         |
| ADD    |        [ x ]        |         [ x ]         |
| SUB    |        [ x ]        |         [ x ]         |
| DIV    |        [ x ]        |         [ x ]         |
| FDIV   |        [ x ]        |         [ x ]         |
| SQRT   |        [ x ]        |         [ x ]         |
| TRNC   |        [ x ]        |         [ x ]         |