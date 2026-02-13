#include "TOF_Attributes.h"

ClassImp( TOF_Attributes );

///////////////////////////////////////////////////////////////////////////////
/// Graph Attrib
///////////////////////////////////////////////////////////////////////////////
void TOF_Attributes::attribGraph( TGraph* g, int color, int marker, int lineSty, double alpha, int lineW )
{
	g->SetMarkerColorAlpha(color, alpha);
	g->SetMarkerStyle(marker);
	g->SetLineStyle(lineSty);
	g->SetLineWidth( lineW );
	g->SetLineColorAlpha(color, alpha);
	g->GetYaxis()->SetTitleOffset(1.3);
}
void TOF_Attributes::attribGraph( TGraph* g )
{
	int    color   = fPrettyGreen;
	double alpha   = fPrettyAlpha_graph;
	int    marker  = 20;
	int    lineSty = 1;
	int    lineW   = 1;

	g->SetMarkerColorAlpha(color, alpha);
	g->SetMarkerStyle(marker);
	g->SetLineStyle(lineSty);
	g->SetLineWidth( lineW );
	g->SetLineColorAlpha(color, alpha);
	g->GetYaxis()->SetTitleOffset(1.3);
}
void TOF_Attributes::attribGraphErrors( TGraphErrors* g, int color, int marker, int lineSty, double alpha, int lineW )
{
	g->SetMarkerColorAlpha(color, alpha);
	g->SetMarkerStyle(marker);
	g->SetLineStyle(lineSty);
	g->SetLineWidth( lineW );
	g->SetLineColorAlpha(color, alpha);
	g->GetYaxis()->SetTitleOffset(1.3);
}
void TOF_Attributes::attribGraphErrors( TGraphErrors* g )
{
	int    color   = fPrettyGreen;
	double alpha   = fPrettyAlpha_graph;
	int    marker  = 20;
	int    lineSty = 1;
	int    lineW   = 1;

	g->SetMarkerColorAlpha(color, alpha);
	g->SetMarkerStyle(marker);
	g->SetLineStyle(lineSty);
	g->SetLineWidth( lineW );
	g->SetLineColorAlpha(color, alpha);
	g->GetYaxis()->SetTitleOffset(1.3);
}
///////////////////////////////////////////////////////////////////////////////
/// Hist Attrib
///////////////////////////////////////////////////////////////////////////////
void TOF_Attributes::attribHist( TH1* hist, int color, double alpha, int fill )
{
  hist->SetMarkerStyle( 24 );
  hist->SetMarkerSize ( 0.6 );
  hist->SetMarkerColor( color );
  hist->SetLineColor( color );
  hist->SetFillStyle( fill ); 
  hist->SetFillColorAlpha( color, alpha);
	
	hist->GetYaxis()->SetTitleOffset(1.3);
}

void TOF_Attributes::attribHist( TH1* hist )
{
	int    color = fPrettyGreen;
	double alpha = fPrettyAlpha_hist;
	int    fill  = fFillSolid;
  hist->SetMarkerStyle( 24 );
  hist->SetMarkerSize ( 0.6 );
  hist->SetMarkerColor( color );
  hist->SetLineColor( color );
  hist->SetFillStyle( fill ); 
  hist->SetFillColorAlpha( color, alpha );
	
	hist->GetYaxis()->SetTitleOffset(1.3);
}

void TOF_Attributes::attribTH2( TH2* hist, int color, int fill )
{
	hist->GetYaxis()->SetTitleOffset(1.3);
  hist->SetLineColor( color );
  hist->SetMarkerColor( color );
  hist->SetFillStyle( fill ); // 1000 = fill, 0=hollow	
  hist->SetFillColorAlpha( color, 0.3 );

	hist->GetYaxis()->SetTitleOffset(1.3);
}

void TOF_Attributes::attribProf( TProfile* prof, int color )
{
	if( color>=0 ) {
	prof->SetLineColor( color );
	prof->SetMarkerColor( color );
	}
	prof->GetYaxis()->SetTitleOffset(1.3);
}

///////////////////////////////////////////////////////////////////////////////
/// Axis Attrib
///////////////////////////////////////////////////////////////////////////////
void TOF_Attributes::setTimeXaxis( TGraph* g, bool kUTC ) {
	g->GetXaxis()->SetTimeDisplay(1);
	g->GetXaxis()->SetTimeFormat ("%m/%d %H:%M");
	if( kUTC ) g->GetXaxis()->SetTimeOffset(0, "gmt");
}
void TOF_Attributes::setTimeXaxis( TGraph* g )
{ 
  g->GetXaxis()->SetTimeDisplay(1);
  g->GetXaxis()->SetTimeFormat( "%m/%d %H:%M" );
  g->GetXaxis()->SetTimeOffset(0, "gmt" );
  
  return;
}
void TOF_Attributes::setTimeXaxis( TMultiGraph* g, double tmin, double tmax )
{
  g->GetXaxis()->SetTimeDisplay(1);
  g->GetXaxis()->SetTimeFormat( "%m/%d %H:%M" );
  g->GetXaxis()->SetTimeOffset(0, "gmt" );

	if( tmin==0 && tmax==0 ) return;

	g->GetXaxis()->SetRangeUser( tmin, tmax );
}
void TOF_Attributes::attribYaxis( TMultiGraph* g, double labelSize, double titleOffset, double titleSize, double ymin, double ymax )
{
  g->GetYaxis()->SetLabelSize  (labelSize  );
  g->GetYaxis()->SetTitleOffset(titleOffset);
  g->GetYaxis()->SetTitleSize  (titleSize  );
  //g->GetYaxis()->SetTitleFont  ( 62 );
  g->GetYaxis()->CenterTitle   (true);
	
	if( ymin==0 && ymax==0 ) return;
	g->GetYaxis()->SetRangeUser( ymin, ymax );

	return;
}
void TOF_Attributes::attribYaxis( TGraph* g, double labelSize, double titleOffset, double titleSize, double ymin, double ymax )
{
  g->GetYaxis()->SetLabelSize  (labelSize  );
  g->GetYaxis()->SetTitleOffset(titleOffset);
  g->GetYaxis()->SetTitleSize  (titleSize  );
  //g->GetYaxis()->SetTitleFont  ( 62 );
  g->GetYaxis()->CenterTitle   (true);
	
	if( ymin==0 && ymax==0 ) return;
	g->GetYaxis()->SetRangeUser( ymin, ymax );

	return;
}


///////////////////////////////////////////////////////////////////////////////
/// StatBox Attrib
///////////////////////////////////////////////////////////////////////////////
void TOF_Attributes::moveStatBox( TPaveStats* ps, double x0, double y0, double x1, double y1 )
{
	if( x0<1 && y0<1 && x1<1 && y1<1 && x0>0 && y0>0 && x1>0 && y1>0 )
	{
	  ps->SetX1NDC( x0 );
	  ps->SetX2NDC( x1 );
	  ps->SetY1NDC( y0 );
	  ps->SetY2NDC( y1 );
	}
	else {
	  ps->SetX1( x0 );
	  ps->SetX2( x1 );
	  ps->SetY1( y0 );
	  ps->SetY2( y1 );
	}

	return;
}

int TOF_Attributes::moveStatBoxNDC( TPaveStats* ps, double x0, double y0, double x1, double y1 )
{
	if( x0<1 && x0>0 ) ps->SetX1NDC( x0 );
	else return TOF_ERR;

	if( y0<1 && y0>0 ) ps->SetY1NDC( y0 );
	else return TOF_ERR;

	if( x1<1 && x1>0 ) ps->SetX2NDC( x1 );
	else return TOF_ERR;

	if( y1<1 && y1>0 ) ps->SetY2NDC( y1 );
	else return TOF_ERR;

	return 1;
}


///////////////////////////////////////////////////////////////////////////////
/// TLegend Attrib
///////////////////////////////////////////////////////////////////////////////
/// Set colors of TLegend entry colors
/// colorOpt = "marker", "line", or "fill"
void TOF_Attributes::setAttribLegendEntries(TLegend *leg, std::string colorOpt)
{
  if(!leg) return;
	TList* l = leg->GetListOfPrimitives();
	TIter next( (TCollection*) l);
	while (auto entry = (TLegendEntry *) next())
	{
		Color_t color=kBlack;

	  if( entry->GetObject()->InheritsFrom(TH1::Class()) ) {
	    auto obj = (TH1*) entry->GetObject();
	    if( colorOpt=="marker" ) color = obj->GetMarkerColor();
	    if( colorOpt=="line"   ) color = obj->GetLineColor();
	    if( colorOpt=="fill"   ) color = obj->GetFillColor();
	  }
	  if( entry->GetObject()->InheritsFrom(TH2::Class()) ) {
	    auto obj = (TH1*) entry->GetObject();
	    if( colorOpt=="marker" ) color = obj->GetMarkerColor();
	    if( colorOpt=="line"   ) color = obj->GetLineColor();
	    if( colorOpt=="fill"   ) color = obj->GetFillColor();
	  }
	  if( entry->GetObject()->InheritsFrom(TGraph::Class()) ) {
	    auto obj = (TGraph*) entry->GetObject();
	    if( colorOpt=="marker" ) color = obj->GetMarkerColor();
	    if( colorOpt=="line"   ) color = obj->GetLineColor();
	    if( colorOpt=="fill"   ) color = obj->GetFillColor();
	  }
	  if( entry->GetObject()->InheritsFrom(TGraphErrors::Class()) ) {
	    auto obj = (TGraphErrors*) entry->GetObject();
	    if( colorOpt=="marker" ) color = obj->GetMarkerColor();
	    if( colorOpt=="line"   ) color = obj->GetLineColor();
	    if( colorOpt=="fill"   ) color = obj->GetFillColor();
	  }
	  entry->SetTextColor( color );
	}
}
    
void TOF_Attributes::setAttribLegend(TLegend *leg, double marginSize, double textSize, int fontType) 
{
	if(!leg) return;
  leg->SetFillStyle (0);
  leg->SetBorderSize(0);
  leg->SetTextSize  (textSize);
  leg->SetTextFont  (fontType);
  leg->SetMargin    (marginSize);
}

///////////////////////////////////////////////////////////////////////////////
/// TText Attrib
///////////////////////////////////////////////////////////////////////////////
void TOF_Attributes::drawText( double size, int align, int color, double x0, double y0, const char* tt)
{
	TLatex kTEXT;
	kTEXT.SetTextSize( size );
	kTEXT.SetTextColor( color );
	kTEXT.SetTextAlign( align );
	kTEXT.DrawText( x0, y0, tt );
}
void TOF_Attributes::drawTextNDC( double size, int align, int color, double x0, double y0, const char* tt)
{
	TLatex kTEXT;
	kTEXT.SetTextSize( size );
	kTEXT.SetTextColor( color );
	kTEXT.SetTextAlign( align );
	kTEXT.DrawTextNDC( x0, y0, tt );
}
void TOF_Attributes::drawText( double size, int align, int color, int font, double x0, double y0, const char* tt)
{
	TLatex kTEXT;
	kTEXT.SetTextSize( size );
	kTEXT.SetTextColor( color );
	kTEXT.SetTextAlign( align );
	kTEXT.SetTextFont ( font );
	kTEXT.DrawLatexNDC( x0, y0, tt );
}
void TOF_Attributes::drawTextNDC( double size, int align, int color, int font, double x0, double y0, const char* tt)
{
	TLatex kTEXT;
	kTEXT.SetTextSize( size );
	kTEXT.SetTextColor( color );
	kTEXT.SetTextAlign( align );
	kTEXT.SetTextFont ( font );
	kTEXT.DrawLatexNDC( x0, y0, tt );
}

