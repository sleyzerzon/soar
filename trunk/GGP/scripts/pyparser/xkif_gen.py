import sys
from ply import yacc
import gdllex
import pdb

output = ''
tokens = None

def p_rule_list(p):
	'''rule_list : rule rule_list
	             | empty'''
	pass

def p_rule(p):
	'''rule : atomic_sentence
	        | LPAREN ARROW atomic_sentence condition_list RPAREN'''

	global output

	if len(p) == 2:
		output += 'XXX_RULE_START\n%s\nXXX_RULE_END\n' % p[1].strip()
	else:
		output += 'XXX_RULE_START\n%s\n%s\nXXX_RULE_END\n' % (p[3].strip(), p[4].strip())

def p_atomic_sentence(p):
	'''atomic_sentence : NAME
	                   | LPAREN INITTRUENEXT function RPAREN
					   | LPAREN LEGALDOES NAME function RPAREN
	                   | LPAREN NAME term_list RPAREN'''
	if len(p) == 2:
		p[0] = 'XXX_SENT_BEGIN\n%s\nXXX_SENT_END' % p[1]
	elif len(p) == 5:
		p[0] = 'XXX_SENT_BEGIN\n%s\n%s\nXXX_SENT_END' % (p[2], p[3])
	else:
		# legal, does
		p[0] = 'XXX_SENT_BEGIN\n%s\n%s\n%s\nXXX_SENT_END' % (p[2], p[3], p[4])

def p_term_list(p):
	'''term_list : empty 
	             | term term_list'''
	if len(p) == 3:
		p[0] = '%s\n%s' % (p[1], p[2])
	else:
		p[0] = ''

def p_term_variable(p):
	'term : VAR'
	p[0] = p[1]

def p_term_str_constant(p):
	'term : NAME'
	p[0] = p[1]

def p_term_num_constant(p):
	'term : NUM'
	p[0] = p[1]

def p_term_function(p):
	'term : function'
	p[0] = p[1]

def p_function(p):
	'''function : NAME
	            | LPAREN NAME term_list RPAREN'''
	# a function is a predicate
	if len(p) == 2:
		p[0] = 'XXX_FUNCTION_BEGIN\n%s\nXXX_FUNCTION_END' % p[1]
	else:
		p[0] = 'XXX_FUNCTION_BEGIN\n%s\n%s\nXXX_FUNCTION_END' % (p[2], p[3].strip())

def p_condition_list(p):
	'''condition_list : empty 
	                  | condition condition_list'''
	if len(p) == 3:
		p[0] = '%s\n%s' % (p[1], p[2].strip())
	else:
		p[0] = ''

def p_condition_literal(p):
	'condition : literal'
	p[0] = p[1]

#def p_condition_distinct(p):
#	'condition : distinct'

def p_condition_or(p):
	'condition : LPAREN OR condition_list RPAREN'
	p[0] = 'XXX_OR_BEGIN\n%s\nXXX_OR_END' % p[3]

def p_literal_atomic_sentence(p):
	'literal : atomic_sentence'
	p[0] = p[1]

def p_literal_not(p):
	'literal : LPAREN NOT atomic_sentence RPAREN'
	p[0] = 'XXX_NOT\n%s' % p[3]

#def p_distinct_var(p):
#	'distinct : LPAREN DISTINCT VAR VAR RPAREN'
#
#def p_distinct_str(p):
#	'distinct : LPAREN DISTINCT VAR NAME RPAREN'
#
#def p_distinct_num(p):
#	'distinct : LPAREN DISTINCT VAR NUM RPAREN'

def p_error(t):
	print "error: %s at line %d" % (t.value, t.lexer.lineno)
	#print "Syntax error on line %d" % p.lineno(1)

def p_empty(p):
	'empty : '
	pass

def parse_file(filename):
	global output
	global tokens

	output = ''
	gdllex.lex_file(filename)
	tokens = gdllex.tokens

	yacc.yacc()
	file = open(filename, 'r').read()
	yacc.parse(file)

if __name__ == '__main__':
	parse_file(sys.argv[1])
	print output
