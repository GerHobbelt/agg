//----------------------------------------------------------------------------
// Anti-Grain Geometry (AGG) - Version 2.5
// A high quality rendering engine for C++
// Copyright (C) 2002-2006 Maxim Shemanarev
// Contact: mcseem@antigrain.com
//          mcseemagg@yahoo.com
//          http://antigrain.com
// 
// AGG is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// AGG is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AGG; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
// MA 02110-1301, USA.
//----------------------------------------------------------------------------

#ifndef AGGX_VCGEN_STROKE_INCLUDED
#define AGGX_VCGEN_STROKE_INCLUDED

#include "aggx_math_stroke.h"


namespace aggx
{

	//============================================================vcgen_stroke
	//
	// See Implementation agg_vcgen_stroke.cpp
	// Stroke generator
	//
	//------------------------------------------------------------------------
	class vcgen_stroke
	{
		enum status_e
		{
			initial,
			ready,
			cap1,
			cap2,
			outline1,
			close_first,
			outline2,
			out_vertices,
			end_poly1,
			end_poly2,
			stop
		};

	public:
		typedef vertex_sequence<vertex_dist> vertex_storage;
		typedef std::vector<point_r>         coord_storage;

		vcgen_stroke();

		void line_cap(line_cap_e lc)     { m_stroker.line_cap(lc); }
		void line_join(line_join_e lj)   { m_stroker.line_join(lj); }
		void inner_join(inner_join_e ij) { m_stroker.inner_join(ij); }

		line_cap_e   line_cap()   const { return m_stroker.line_cap(); }
		line_join_e  line_join()  const { return m_stroker.line_join(); }
		inner_join_e inner_join() const { return m_stroker.inner_join(); }

		void width(real w) { m_stroker.width(w); }
		void miter_limit(real ml) { m_stroker.miter_limit(ml); }
		void miter_limit_theta(real t) { m_stroker.miter_limit_theta(t); }
		void inner_miter_limit(real ml) { m_stroker.inner_miter_limit(ml); }
		void approximation_scale(real as) { m_stroker.approximation_scale(as); }

		real width() const { return m_stroker.width(); }
		real miter_limit() const { return m_stroker.miter_limit(); }
		real inner_miter_limit() const { return m_stroker.inner_miter_limit(); }
		real approximation_scale() const { return m_stroker.approximation_scale(); }

		void shorten(real s) { m_shorten = s; }
		real shorten() const { return m_shorten; }

		// Vertex Generator Interface
		void remove_all();
		void add_vertex(real x, real y, unsigned cmd);

		// Vertex Source Interface
		void     rewind(unsigned path_id);
		unsigned vertex(real* x, real* y);

	private:
		vcgen_stroke(const vcgen_stroke&);
		const vcgen_stroke& operator = (const vcgen_stroke&);

		math_stroke<coord_storage> m_stroker;
		vertex_storage m_src_vertices;
		coord_storage m_out_vertices;
		real                     m_shorten;
		unsigned                   m_closed;
		status_e                   m_status;
		status_e                   m_prev_status;
		unsigned                   m_src_vertex;
		unsigned                   m_out_vertex;
	};
}

#endif
