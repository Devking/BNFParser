=begin
	Code by Wells Lucas Santo.
	Written for CS6373 Programming Languages, Spring 2015.

	This code was written for purely academic purposes. 
	Direct reproduction of this code is not permitted.
=end

# General Idea: each Valid function will return [flag, value]
# Where each Valid function represents a non-terminal variable in the CFG
# The flag will carry with it true/false based on whether it fits the CFG
# The value will bring back up the currently calculated value from the expression
# All of this is done in a recursive fashion

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
			flagger = comp1[0]						# Check if section before '*' is valid EXPR
			comp2 = ValidTerm?(string[loc+1..string.length-1])
			flagger &= comp2[0]						# Check if section after '*' is valid TERM
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
			flagger = comp1[0]						# Check if section before '+' is valid EXPR
			comp2 = ValidTerm?(string[loc+1..string.length-1])
			flagger &= comp2[0]						# Check if section after '+' is valid TERM
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
			flagger = comp1[0]						# Check if section before '-' is valid EXPR
			comp2 = ValidTerm?(string[loc+1..string.length-1])
			flagger &= comp2[0]						# Check if section after '-' is valid TERM
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
	string = string.gsub(/\s+/, "")	# strip ALL whitespace (spaces, tabs, newlines) from the string
									# usng regular expressions, denoted by the //
									# \s+ matches any whitespace that occurs one or more times
	return ValidExpr?(string)
end

#INPUT
def checkInput
	# initialize the flags and the output string
	flag = true
	sawend = false
	memory = ""
	# take all input from standard input until EOF is reached
	input = $stdin.read
	# now test to see if the input is valid
	input.each_line do |splitin|
		# check case 2 and case 3 of INPUT
		if (splitin.chomp == 'quit' || splitin.chomp == 'QUIT')
			if (sawend)
				flag = false
			end
			sawend = true
		# check case 1 of INPUT
		else
			if (sawend)
				flag = false
			end
			comp = ValidLine?(splitin)
			flag &= comp[0]
			memory += comp[1].to_s << "\n"	# we append the value to the back of the output string
		end
	end

	# this is the section to decide what to print
	if (flag && sawend) # check that one and only one 'QUIT' was used and that expressions were valid
		puts memory		# if so, print the evaluated numbers!
	else
		puts "ERR"
	end
end

checkInput
