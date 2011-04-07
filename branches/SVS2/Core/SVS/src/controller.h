#include <vector>

typedef struct dim_description_struct {
	double min;
	double max;
	double step;
} dim_description;

typedef std::vector<dim_description> env_output_description;

class env_output {
public:
	env_output(const env_output_description &d)
	: desc(d)
	{
		output_description::iterator i;
		for(i = desc.begin(); i != desc.end(); ++i) {
			value.push_back(i->min);
		}
	}
	
	env_output(const env_output &other) 
	: value(other.value), desc(other.desc)
	{ }
	
	bool increment() {
		int i;
		for (i = 0; i < desc.size(); ++i) {
			if (value[i] + desc[i].step <= desc[i].max) {
				value[i] += desc[i].step;
				return true;
			}
		}
		return false;
	}
	
private:
	std::vector<double> value;
	env_output_description desc;
};

class controller {
public:
	controller(model *m, objective *f, const env_output_description &outdesc)
	: m(m), f(f), outdesc(outdesc) {}
	
	void output(scene *scn, env_output &out) {
		env_output curr(outdesc);
		double eval, best = f.eval(scn, bestout);
		scene *next;
		while (curr.increment()) {
			next = m.predict(scn);
			eval = f.eval(next, curr);
			if (eval > best) {
				out = curr;
				best = eval;
			}
		}
	}
	
private:
	model *m;
	objective *f;
	env_output_description outdesc;
};
