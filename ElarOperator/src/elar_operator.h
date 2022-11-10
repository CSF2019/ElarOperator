#ifndef __ELAR_OPERATOR__
#define __ELAR_OPERATOR__

#include "half_edge_structure.h"
#include <vector>
#include <iostream>

using namespace std;

class ElarOperator
{
public:
	void addEdgeIntoSolid(Edge *edge, Solid *&solid);
	void addFaceIntoSolid(Face *face, Solid *&solid);
	void addLoopIntoFace(Loop *loop, Face *face);
	Solid *mvfs(double point[3], Vertex *&vertex);
	HalfEdge *mev(Vertex *sv, double point[3], Loop *lp);
	Loop *mef(Vertex *sv, Vertex *ev, Loop *lp, bool mark);
	Loop *kemr(Vertex *sv, Vertex *ev, Loop *lp);
	void kfmrh(Face *fa, Face *fb);
	void sweep(double dir[3], double dist);

	ElarOperator()
	{
		v_list.clear();
		sweep_list.clear();
		l_list.clear();
	}

	std::vector<Vertex *> getV_list()
	{
		return v_list;
	}

	std::vector<Face *> getSweep_list()
	{
		return sweep_list;
	}

	std::vector<Loop *> getLoop_list()
	{
		return l_list;
	}

	void printLoop(Loop *loop)
	{

		if (loop == NULL)return;
		HalfEdge *ha = loop->halfedges;
		Vertex *vertex = ha->sv;

		int cnt = 1;
		ha = ha->next;
		while (ha->sv != vertex)
		{
			++cnt;
			ha = ha->next;
		}
		cnt += 1;
		cout << " " << vertex->id;
		ha = ha->next;

		while (ha->sv != vertex)
		{
			cout << " " << ha->sv->id;
			ha = ha->next;
		}

		cout << " " << vertex->id << endl;
	}
private:
	std::vector<Vertex *> v_list;
	std::vector<Loop *> l_list;
	std::vector<Face *> sweep_list;
};

#endif
