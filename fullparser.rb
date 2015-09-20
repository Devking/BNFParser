=begin
    Code by Wells Lucas Santo
    Modified from my original parser from CS6373, Spring 2015.
=end

#NUMBER
def ValidNumber? (string)                               
    ### Check if a non-numeric character ever appears in the string using /\D/
    ### If so, we know it's not a 'NUMBER' as defined
    ### Also check if string is empty
    return ( not(/\D/ =~ string) && not(string.length.zero?) ), string.to_i
end

#PEXPR
def ValidPexpr? (string)
    ################################ PEXPR Case 1
    if (string[0] == '(' && string[string.length-1] == ')')
        if (string.length < 3) #Fail if "(EXPR)" length not met
            return false, 0
        end
        compute = ValidExpr?(string[1..string.length-2]) #Recurse for "EXPR"
        valid = compute[0]
        value = compute[1]
        return valid, value
    ################################### PEXPR Case 2
    elsif (string[0] == '-')
        if (string.length < 1)
            return false, 0
        end
        compute = ValidPexpr?(string[1..string.length-1]) #Recurse for "PEXPR"
        return compute[0], -compute[1]
    ################################### PEXPR Case 3
    else
        return ValidNumber?(string)
    end
end

#TERM
def ValidTerm? (string)
    if (string.length.zero?)
        return false, 0
    end
    ### Case 1
    # Check if a '*' exists in the expression
    # If there are many '*', there may be different places to break it up
    # If only one is valid, then we will only send that computation back up
    # If none are valid, then we will fail the expression
    # If multiple breaks are valid, we will take precedence from left to right
    if not(string.index('*').nil?)
        nosub = string.count('*')
        lastsub = 0
        for i in 1..nosub
            loc = string.index('*', lastsub)
            lastsub = loc+1
            if (loc == 0)
                return false, 0
            end
            if (loc == string.length-1)
                return false, 0
            end
            comp1 = ValidExpr?(string[0..loc-1])
            flagger = comp1[0]                      # Check if section before '*' is valid EXPR
            comp2 = ValidTerm?(string[loc+1..string.length-1])
            flagger &= comp2[0]                     # Check if section after '*' is valid TERM
            if (flagger)
                return flagger, (comp1[1] * comp2[1])
            end
        end
    end
    ### Case 2
    # If there was no '*', the only alternative is that this is a PEXPR
    return ValidPexpr?(string)
end

#EXPR
def ValidExpr? (string)
    if (string.length.zero?)
        return false, 0
    end
    #### Case 1
    # Check if '+' exists in the expression
    # Follows the same idea as I did with '*' above
    if not(string.index('+').nil?)
        nosub = string.count('+')
        lastsub = 0
        for i in 1..nosub
            loc = string.index('+', lastsub)
            lastsub = loc+1
            if (loc == 0)
                return false, 0
            end
            if (loc == string.length-1)
                return false, 0
            end
            comp1 = ValidExpr?(string[0..loc-1])
            flagger = comp1[0]                      # Check if section before '+' is valid EXPR
            comp2 = ValidTerm?(string[loc+1..string.length-1])
            flagger &= comp2[0]                     # Check if section after '+' is valid TERM
            if (flagger)
                return flagger, (comp1[1]+comp2[1])
            end
        end
    end
    #### Case 2
    # Check if '-' exists in the expression
    # Follows the same idea as I did with '+' above
    if not(string.index('-').nil?)
        nosub = string.count('-')
        lastsub = 0
        for i in 1..nosub
            loc = string.index('-', lastsub)
            if (loc == 0) # this occurs when we actually are trying to do a negation
                break
            end
            lastsub = loc+1
            if (loc == 0)
                return false, 0
            end
            if (loc == string.length-1)
                return false, 0
            end
            comp1 = ValidExpr?(string[0..loc-1])
            flagger = comp1[0]                      # Check if section before '-' is valid EXPR
            comp2 = ValidTerm?(string[loc+1..string.length-1])
            flagger &= comp2[0]                     # Check if section after '-' is valid TERM
            if (flagger)
                return flagger, (comp1[1]-comp2[1])
            end
        end
    end
    #### Case 3
    # If none of the above cases returned true, then the only alternative is 'TERM'
    return ValidTerm?(string)
end

#LINE
def ValidLine? (string)
    string = string.gsub(/\s+/, "") # strip ALL whitespace (spaces, tabs, newlines) from the string
                                    # usng regular expressions, denoted by the //
                                    # \s+ matches any whitespace that occurs one or more times
    return ValidExpr?(string)
end

def main
    puts "--------------------------------------------------"
    puts "Calculator Application 1.0"
    puts "Enter a basic arithmetic calculation and hit Enter"
    puts "Hit Ctrl+D to quit"
    puts "--------------------------------------------------"
    while line = gets
        comp = ValidLine?(line)
        if (comp[0])
            puts comp[1]
        else
            puts "Line formatted incorrectly"
        end
    end
end

main
