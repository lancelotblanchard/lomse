//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2012 Cecilio Salmeron. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice, this
//      list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright notice, this
//      list of conditions and the following disclaimer in the documentation and/or
//      other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
// SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// For any comment, suggestion or feature request, please contact the manager of
// the project at cecilios@users.sourceforge.net
//---------------------------------------------------------------------------------------

#include <UnitTest++.h>
#include <sstream>
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_ldp_exporter.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_ldp_compiler.h"
#include "lomse_document.h"
#include "lomse_im_factory.h"


using namespace UnitTest;
using namespace std;
using namespace lomse;


//=======================================================================================
// test for traversing the Internal Model with Visitor objects
//=======================================================================================
#define k_show_tree    0    //set to 1 for visualizing the visited tree

class MyVisitor
{
protected:
    LibraryScope& m_libraryScope;
    int m_indent;
    int m_nodesIn;
    int m_nodesOut;
    int m_maxDepth;

public:
    MyVisitor(LibraryScope& libraryScope)
        : m_libraryScope(libraryScope), m_indent(0), m_nodesIn(0)
        , m_nodesOut(0), m_maxDepth(0) {}
	virtual ~MyVisitor() {
#if (k_show_tree == 1)
	    cout << "---------------------------------------------------------" << endl;
#endif
	}

    int num_in_nodes() { return m_nodesIn; }
    int num_out_nodes() { return m_nodesOut; }
    int max_depth() { return m_maxDepth; }

    void start_visiting(ImoObj* pImo)
    {
        int type = pImo->get_obj_type();
        const string& name = pImo->get_name();
#if (k_show_tree == 0)
        if (name == "unknown")
        {
#endif
            if (pImo->has_visitable_children())
            {
                cout << indent() << "(" << name << " type " << type
                     << ", id=" << pImo->get_id() << endl;

            }
            else
            {
                cout << indent() << "(" << name << " type " << type
                     << ", id=" << pImo->get_id() << ")" << endl;
            }
#if (k_show_tree == 0)
        }
#endif

        m_indent++;
        m_nodesIn++;
        if (m_maxDepth < m_indent)
            m_maxDepth = m_indent;
    }

	void end_visiting(ImoObj* pImo)
    {
        m_indent--;
        m_nodesOut++;
        if (!pImo->has_visitable_children())
            return;
#if (k_show_tree == 1)
        cout << indent() << ")" << endl;
#endif
    }

    string indent()
    {
        string spaces = "";
        for (int i=0; i < 3*m_indent; ++i)
            spaces += " ";
        return spaces;
    }

};

//---------------------------------------------------------------------------------------
class MyObjVisitor : public Visitor<ImoObj>, public MyVisitor
{
public:
    MyObjVisitor(LibraryScope& libraryScope)
        : Visitor<ImoObj>(), MyVisitor(libraryScope) {}
	~MyObjVisitor() {}

    void start_visit(ImoObj* pImo) { start_visiting(pImo); }
    void end_visit(ImoObj* pImo) { end_visiting(pImo); }
};

//---------------------------------------------------------------------------------------
class MyParaVisitor : public Visitor<ImoParagraph>, public MyVisitor
{
public:
    MyParaVisitor(LibraryScope& libraryScope)
        : Visitor<ImoParagraph>(), MyVisitor(libraryScope) {}
	~MyParaVisitor() {}

    void start_visit(ImoParagraph* pImo) { start_visiting(pImo); }
    void end_visit(ImoParagraph* pImo) { end_visiting(pImo); }
};

//---------------------------------------------------------------------------------------
class MyHPVisitor : public Visitor<ImoParagraph>
                  , public Visitor<ImoHeading>
                  , public MyVisitor
{
public:
    MyHPVisitor(LibraryScope& libraryScope)
        : Visitor<ImoParagraph>(), Visitor<ImoHeading>()
        , MyVisitor(libraryScope) {}
	~MyHPVisitor() {}

    void start_visit(ImoParagraph* pImo) { start_visiting(pImo); }
    void start_visit(ImoHeading* pImo) { start_visiting(pImo); }
    void end_visit(ImoParagraph* pImo) { end_visiting(pImo); }
    void end_visit(ImoHeading* pImo) { end_visiting(pImo); }
};


//---------------------------------------------------------------------------------------
class ImVisitorTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    ImVisitorTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_scores_path = LOMSE_TEST_SCORES_PATH;
    }

    ~ImVisitorTestFixture()    //TearDown fixture
    {
    }

};

SUITE(ImVisitorTest)
{
    TEST_FIXTURE(ImVisitorTestFixture, Document)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 2.3) (content))" );
        ImoDocument* pRoot = doc.get_imodoc();

        MyObjVisitor v(m_libraryScope);
        pRoot->accept_visitor(v);

//        cout << "max_depth=" << v.max_depth()
//             << ", num_in_nodes=" << v.num_in_nodes()
//             << ", num_out_nodes=" << v.num_out_nodes() << endl;
        CHECK( v.max_depth() == 3 );
        CHECK( v.num_in_nodes() == 4 );
        CHECK( v.num_out_nodes() == v.num_in_nodes() );
    }

    TEST_FIXTURE(ImVisitorTestFixture, EbookExample)
    {
        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "60004-ebook-example.lms" );
        ImoDocument* pRoot = doc.get_imodoc();

        MyObjVisitor v(m_libraryScope);
        pRoot->accept_visitor(v);

        //cout << "max_depth=" << v.max_depth()
        //     << ", num_in_nodes=" << v.num_in_nodes()
        //     << ", num_out_nodes=" << v.num_out_nodes() << endl;
        CHECK( v.max_depth() == 9 );
        CHECK( v.num_in_nodes() == 91 );
        CHECK( v.num_out_nodes() == v.num_in_nodes() );
    }

//    TEST_FIXTURE(ImVisitorTestFixture, EbookExample2)
//    {
//        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "60005-ebook-three-pages.lms" );
//        ImoDocument* pRoot = doc.get_imodoc();
//
//        MyObjVisitor v(m_libraryScope);
//        pRoot->accept_visitor(v);
//
//        cout << "max_depth=" << v.max_depth()
//             << ", num_in_nodes=" << v.num_in_nodes()
//             << ", num_out_nodes=" << v.num_out_nodes() << endl;
//        CHECK( v.max_depth() == 9 );
//        CHECK( v.num_in_nodes() == 1321 );
//        CHECK( v.num_out_nodes() == v.num_in_nodes() );
//    }

    TEST_FIXTURE(ImVisitorTestFixture, ParagraphVisitor)
    {
        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "60004-ebook-example.lms" );
        ImoDocument* pRoot = doc.get_imodoc();

        MyParaVisitor v(m_libraryScope);
        pRoot->accept_visitor(v);

//        cout << "max_depth=" << v.max_depth()
//             << ", num_in_nodes=" << v.num_in_nodes()
//             << ", num_out_nodes=" << v.num_out_nodes() << endl;
        CHECK( v.max_depth() == 1 );
        CHECK( v.num_in_nodes() == 4 );
        CHECK( v.num_out_nodes() == v.num_in_nodes() );
    }

    TEST_FIXTURE(ImVisitorTestFixture, ParagraphHeadingVisitor)
    {
        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "60004-ebook-example.lms" );
        ImoDocument* pRoot = doc.get_imodoc();

        MyHPVisitor v(m_libraryScope);
        pRoot->accept_visitor(v);

//        cout << "max_depth=" << v.max_depth()
//             << ", num_in_nodes=" << v.num_in_nodes()
//             << ", num_out_nodes=" << v.num_out_nodes() << endl;
        CHECK( v.max_depth() == 1 );
        CHECK( v.num_in_nodes() == 6 );
        CHECK( v.num_out_nodes() == v.num_in_nodes() );
    }

};



//=======================================================================================
// test for LdpExporter
//=======================================================================================

class LdpExporterTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    LdpExporterTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_scores_path = LOMSE_TEST_SCORES_PATH;
    }

    ~LdpExporterTestFixture()    //TearDown fixture
    {
    }

};

//---------------------------------------------------------------------------------------
SUITE(LdpExporterTest)
{
//    TEST_FIXTURE(LdpExporterTestFixture, visual)
//    {
//        //visual test to display the exported score
//
//        Document doc(m_libraryScope);
////        doc.from_file(m_scores_path + "00205-multimetric.lms" );
////        doc.from_file(m_scores_path + "80120-fermatas.lms" );
//        doc.from_file(m_scores_path + "80051-tie-bezier.lms" );
////        doc.from_file(m_scores_path + "00110-triplet-against-5-tuplet-4.14.lms" );
////        doc.from_file(m_scores_path + "80130-metronome.lms" );
////        doc.from_file(m_scores_path + "80180-new-system-tag.lms" );
////        doc.from_file(m_scores_path + "80110-graphic-line-text.lms" );
////        doc.from_file("/datos/USR/Desarrollo_wx/lomse/samples/chopin_prelude20_v16.lms" );
//        ImoDocument* pRoot = doc.get_imodoc();
//
//        LdpExporter exporter(&m_libraryScope);
//        string source = exporter.get_source(pRoot);
//        cout << "----------------------------------------------------" << endl;
//        cout << source << endl;
//        cout << "----------------------------------------------------" << endl;
//    }

    // clef ------------------------------------------------------------------------------------

//    TEST_FIXTURE(LdpExporterTestFixture, clef)
//    {
//        Document doc(m_libraryScope);
//        ImoClef* pClef = static_cast<ImoClef*>(ImFactory::inject(k_imo_clef, &doc));
//        pClef->set_clef_type(k_clef_F4);
//        LdpExporter exporter(&m_libraryScope);
//        string source = exporter.get_source(pClef);
//        //cout << "\"" << source << "\"" << endl;
//        CHECK( source == "(clef F4 p1)" );
//        delete pClef;
//    }
//
//    // lenmusdoc ----------------------------------------------------------------------------------
//
//    TEST_FIXTURE(LdpExporterTestFixture, lenmusdoc_empty)
//    {
//        Document doc(m_libraryScope);
//        ImoDocument* pImoDoc = static_cast<ImoDocument*>(
//                                        ImFactory::inject(k_imo_document, &doc));
//        pImoDoc->set_version("2.3");
//        LdpExporter exporter(&m_libraryScope);
//        string source = exporter.get_source(pImoDoc);
////        cout << source << endl;
//        CHECK( source ==
//            "(lenmusdoc (vers 2.3)\n"
//            "   //LDP file generated by Lomse, version \n"
//            "   (content\n"
//            "))\n"
//        );
//        delete pImoDoc;
//    }
//
//    TEST_FIXTURE(LdpExporterTestFixture, ErrorNotImplemented)
//    {
//        Document doc(m_libraryScope);
//        ImoTie* pTie = static_cast<ImoTie*>(ImFactory::inject(k_imo_tie, &doc));
//        LdpExporter exporter(&m_libraryScope);
//        string source = exporter.get_source(pTie);
////        cout << source << endl;
//        CHECK( source == "(TODO: tie   type=76, id=0 )\n" );
//        delete pTie;
//    }
//
//    // score ----------------------------------------------------------------------------
//
//    TEST_FIXTURE(LdpExporterTestFixture, score)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string(
//            "(lenmusdoc (vers 0.0) (content (score (vers 1.6)"
//            "(instrument (musicData (clef G)(key D)(n c4 q)(barline) ))"
//            ")))" );
//        ImoScore* pScore = doc.get_score(0);
//        LdpExporter exporter(&m_libraryScope);
//        string source = exporter.get_source(pScore);
//        cout << "\"" << source << "\"" << endl;
//        CHECK( source ==
//              "(score (vers 1.6)\n"
//              "   (instrument \n"
//              "      (musicData \n"
//              "         (clef G p1)\n"
//              "         (key D)\n"
//              "         (n c4 q p1)\n"
//              "         (barline )\n"
//              ")\n)\n)" );
//    }
//
//    // note ------------------------------------------------------------------------------------
//
//    TEST_FIXTURE(LdpExporterTestFixture, Note)
//    {
//        Document doc(m_libraryScope);
//        ImoNote* pImo = ImFactory::inject_note(&doc, k_step_D, k_octave_4,
//                            k_eighth, k_no_accidentals);
//        LdpExporter exporter(&m_libraryScope);
//        string source = exporter.get_source(pImo);
//        //cout << "\"" << source << "\"" << endl;
//        CHECK( source == "(n d4 e p1)" );
//        delete pImo;
//    }
//
//    TEST_FIXTURE(LdpExporterTestFixture, Note_dots)
//    {
//        Document doc(m_libraryScope);
//        ImoNote* pImo = ImFactory::inject_note(&doc, k_step_B, k_octave_7,
//                            k_whole, k_sharp, 2);
//        LdpExporter exporter(&m_libraryScope);
//        string source = exporter.get_source(pImo);
//        //cout << "\"" << source << "\"" << endl;
//        CHECK( source == "(n +b7 w.. p1)" );
//        delete pImo;
//    }
//
//
//    // color ------------------------------------------------------------------------------------
//
//    TEST_FIXTURE(LdpExporterTestFixture, color)
//    {
//        Document doc(m_libraryScope);
//        ImoClef* pImo = static_cast<ImoClef*>(ImFactory::inject(k_imo_clef, &doc));
//        pImo->set_clef_type(k_clef_F4);
//        pImo->set_color( Color(127, 40, 12, 128) );
//        LdpExporter exporter(&m_libraryScope);
//        string source = exporter.get_source(pImo);
////        cout << "\"" << source << "\"" << endl;
//        CHECK( source == "(clef F4 p1 (color #7f280c80))" );
//        delete pImo;
//    }
//
////    // user location ----------------------------------------------------------------------------
////
////    TEST_FIXTURE(LdpExporterTestFixture, ExportLdp_user_location)
////    {
////        ImoClef obj;
////        obj.set_type(k_clef_G2);
////        obj.set_user_location_x(30.0f);
////        obj.set_user_location_y(-7.05f);
////        LdpExporter exporter(&m_libraryScope);
////        string source = exporter.get_source(&obj);
////        cout << "\"" << source << "\"" << endl;
////        CHECK( source == "(clef G3 p1 (dx 30.0000) (dy -7.0500))" );
////    }

};
