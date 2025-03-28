/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmXMLWriter.h"

#include <cassert>

#include "cmsys/FStream.hxx"

cmXMLWriter::cmXMLWriter(std::ostream& output, std::size_t level)
  : Output(output)
  , IndentationElement(1, '\t')
  , Level(level)
{
}

cmXMLWriter::~cmXMLWriter()
{
  assert(this->Indent == 0);
}

void cmXMLWriter::StartDocument(char const* encoding)
{
  this->Output << R"(<?xml version="1.0" encoding=")" << encoding << "\"?>";
}

void cmXMLWriter::EndDocument()
{
  assert(this->Indent == 0);
  this->Output << '\n';
}

void cmXMLWriter::StartElement(std::string const& name)
{
  this->CloseStartElement();
  this->ConditionalLineBreak(!this->IsContent);
  this->Output << '<' << name;
  this->Elements.push(name);
  ++this->Indent;
  this->ElementOpen = true;
  this->BreakAttrib = false;
}

void cmXMLWriter::EndElement()
{
  assert(this->Indent > 0);
  --this->Indent;
  if (this->ElementOpen) {
    this->Output << "/>";
  } else {
    this->ConditionalLineBreak(!this->IsContent);
    this->IsContent = false;
    this->Output << "</" << this->Elements.top() << '>';
  }
  this->Elements.pop();
  this->ElementOpen = false;
}

void cmXMLWriter::Element(char const* name)
{
  this->CloseStartElement();
  this->ConditionalLineBreak(!this->IsContent);
  this->Output << '<' << name << "/>";
}

void cmXMLWriter::BreakAttributes()
{
  this->BreakAttrib = true;
}

void cmXMLWriter::Comment(char const* comment)
{
  this->CloseStartElement();
  this->ConditionalLineBreak(!this->IsContent);
  this->Output << "<!-- " << comment << " -->";
}

void cmXMLWriter::CData(std::string const& data)
{
  this->PreContent();
  this->Output << "<![CDATA[" << data << "]]>";
}

void cmXMLWriter::Doctype(char const* doctype)
{
  this->CloseStartElement();
  this->ConditionalLineBreak(!this->IsContent);
  this->Output << "<!DOCTYPE " << doctype << ">";
}

void cmXMLWriter::ProcessingInstruction(char const* target, char const* data)
{
  this->CloseStartElement();
  this->ConditionalLineBreak(!this->IsContent);
  this->Output << "<?" << target << ' ' << data << "?>";
}

void cmXMLWriter::FragmentFile(char const* fname)
{
  this->CloseStartElement();
  cmsys::ifstream fin(fname, std::ios::in | std::ios::binary);
  this->Output << fin.rdbuf();
}

void cmXMLWriter::SetIndentationElement(std::string const& element)
{
  this->IndentationElement = element;
}

void cmXMLWriter::ConditionalLineBreak(bool condition)
{
  if (condition) {
    this->Output << '\n';
    for (std::size_t i = 0; i < this->Indent + this->Level; ++i) {
      this->Output << this->IndentationElement;
    }
  }
}

void cmXMLWriter::PreAttribute()
{
  assert(this->ElementOpen);
  this->ConditionalLineBreak(this->BreakAttrib);
  if (!this->BreakAttrib) {
    this->Output << ' ';
  }
}

void cmXMLWriter::PreContent()
{
  this->CloseStartElement();
  this->IsContent = true;
}

void cmXMLWriter::CloseStartElement()
{
  if (this->ElementOpen) {
    this->ConditionalLineBreak(this->BreakAttrib);
    this->Output << '>';
    this->ElementOpen = false;
  }
}
