#ifndef TEST_INTERFACE_H
#define TEST_INTERFACE_H

#include <vector>
#include <list>
#include <string>

typedef int wme_hnd;
typedef int sym_hnd;
typedef std::pair<sym_hnd, wme_hnd> sym_wme_pair;

typedef struct wme_struct {
	std::string id;
	std::string attr;
	std::string val;
} wme;

class test_interface {
public:
	test_interface();

	wme_hnd      make_str_wme(sym_hnd id, std::string attr, std::string val);
	sym_wme_pair make_id_wme(sym_hnd id, std::string attr);
	void         remove_wme(wme_hnd w);
	sym_hnd      get_scene_root();
	
	bool         get_env_line(std::string &line);
	
	void         add_line(std::string &line);

private:
	int                    idcounter;
	std::vector<wme>       wmes;
	std::list<std::string> input_lines;
};

typedef test_interface soar_interface;
#endif

