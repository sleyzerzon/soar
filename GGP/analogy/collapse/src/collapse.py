import sys, os
from GDL import *
import gdlyacc
from PositionIndex import PositionIndex

# grounded non-elab rules
g_rules = [] # head pred -> [rule]
# ungrounded state update rules
ug_rules = [] # [rule]

# grounded elab rules
g_elabs = {} # pred name -> [rule]
# ungrounded elab rules
ug_elabs = {} # pred name -> [rule]

def sentence_grounded(sentence):
	return sentence.get_relation() in ['true', 'does']

def rule_grounded(rule):
	for b in rule.get_body():
		if not sentence_grounded(b):
			return False
	return True

# let's hold off on this for now
#
#def negate_body(r):
#
#	binding_sets = {}
#	for b in r.get_body():
#		if b.is_comparison():
#			binding_set = []
#			vars = b.get_var_terms()
#			for v in vars:
#				# find all conditions that contain the variable
#				for b2 in r.get_body():
#					if not b2.is_comparison() and b2.has_term(v):
#						if b2 not in binding_set:
#							binding_set.append(b2)

def expand(r, bi):
	global g_elabs

	expanded = []
	b = r.get_body()[bi]
	pred = b.get_predicate()
	assert pred in g_elabs

	for elab in g_elabs[pred]:
		r_copy = r.copy()
		elab_copy = elab.copy()
		expanded.extend(expand_with_rule(r_copy, bi, elab_copy))
	
	return expanded

def expand_with_rule(r, bi, elab):
	expanded = []
	b = r.get_body()[bi]
	elab_head = elab.get_head()
	positions = PositionIndex.get_all_positions(b)
	assert positions == PositionIndex.get_all_positions(elab_head)
	terms = [p.fetch(b) for p in positions]

	# this is so there won't be name collisions
	elab.mangle_vars("__cr_")
	
	# first we have to make the head of the elab rule and the condition being
	# expanded look exactly the same
	preserve_vars = []
	for p, t in zip(positions, terms):
		elab_term = p.fetch(elab_head)
		if isinstance(elab_term, Constant) and isinstance(t, Variable):
			p.set(b, elab_term.copy())
		elif isinstance(elab_term, Variable) and isinstance(t, Constant):
			p.set(elab_head, t.copy())
		elif isinstance(elab_term, Variable) and isinstance(t, Variable):
			# should change the variables in the elab rule to match the expanded rule
			p.set(elab_term, t.copy())
			preserve_vars.append(t.get_name())
		else:
			assert t == elab_term

	r.enforce_equality(preserve_vars)
	elab.enforce_equality(preserve_vars)
	
	offset = r.num_conditions()
	gr_head = elab.get_head()

	# for each constraint that applied to the condition 
	# being expanded, we have to insert duplicates of 
	# those that apply to the inserted conditions

	# we're going to append all the new rules to the end of
	# the body first, and then add and modify the approriate
	# constraints. After everything is done, remove the body
	# condition that was substituted
	
	# no variable equalities should be drawn here
	for grb in elab.get_body():
		r.add_condition(grb)

	# add the variable constraints
	for p in positions:
		if isinstance(p.fetch(gr_head), Constant):
			continue
		b2b_cons = r.get_constraints_on(bi, p) # body-to-body constraints
		hv_bindings = elab.get_headvar_binding(p)
		for old_i, old_p, comp, order in b2b_cons:
			for bound_i, bound_p in hv_bindings:
				# head var first
				if order == 0:
					r.add_pos_constraint(offset + bound_i, bound_p, old_i, old_p, comp)
				else:
					r.add_pos_constraint(old_i, old_p, offset + bound_i, bound_p, comp)

				if comp.relation() == '==':
					# have to rename the variables in original condition to
					# equal new condition
					new_cond = r.get_cond(bound_i + offset)
					new_name = bound_p.fetch(new_cond)
					old_p.set(r.get_cond(old_i), new_name)

	# finally remove the replaced condition
	r.remove_condition(bi)
	expanded.append(r)

	return expanded

def can_expand(r):
	"""Return the index of a condition in r that can be expanded given the current
	   set of grounded elaborations. If no such condition exists, return -1"""
	global g_elabs
	global ug_elabs

	for bi, b in enumerate(r.get_body()):
		if b.is_negated():
			# ignore negations for now
			continue
		if not sentence_grounded(b):
			elab_rel = b.get_relation()
			if elab_rel in g_elabs and elab_rel not in ug_elabs:
				# only those elaborations for which all generating rules are
				# grounded can be expanded. Otherwise, we might miss some
				# instantiations
				return bi
	
	# can't expand any sentences
	return -1


def collapse(ir):
	global g_rules
	global ug_rules
	global g_elabs
	global ug_elabs

	# first split the rules into grounded and ungrounded
	for r in ir.get_update_rules() + ir.get_goal_rules() + ir.get_terminal_rules():
		if len(r.get_body()) == 0:
			continue

		if not rule_grounded(r):
			ug_rules.append(r)
		else:
			g_rules.append(r)
	
	for r in ir.get_elab_rules():
		if len(r.get_body()) == 0:
			continue
	
		pred = r.get_head().get_predicate()
		if not rule_grounded(r):
			ug_elabs.setdefault(pred,[]).append(r)
		else:
			g_elabs.setdefault(pred,[]).append(r)

	# the main loops

	# first try to ground all the elab rules, since grounding update, goal, or
	# terminal rules has no effect on the grounding of other rules
	while len(ug_elabs) > 0:
		for p, rules in ug_elabs.items():
			for r in rules[:]:
				expand_bi = can_expand(r)
				if expand_bi >= 0:
					expanded = expand(r, expand_bi)
					rules.remove(r)
					for er in expanded:
						if rule_grounded(er):
							g_elabs.setdefault(p,[]).append(er)
						else:
							rules.append(er)
					if len(rules) == 0:
						del ug_elabs[p]
	
	# at this point, all elaboration rules are grounded. We should be able to
	# go through all the other kinds of ungrounded rules
	while len(ug_rules) > 0:
		for r in ug_rules[:]:
			expand_bi = can_expand(r)
			assert expand_bi >= 0, "Rule is impossible to expand"
			expanded = expand(r, expand_bi)
			ug_rules.remove(r)
			ug_rules.extend(expanded)

	# all finished
	return g_rules
	

if __name__ == '__main__':
	gdlyacc.parse_file(sys.argv[1])
	grounded = collapse(gdlyacc.int_rep)
	for r in grounded:
		print r
