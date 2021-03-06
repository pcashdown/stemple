// Expander
// Processes input streams using embedded directives and macro expansions.
//
// Copyright � 2016 by Paul Ashdown. All Rights Reserved.

#pragma once
#ifndef __stemple__Expander__
#define __stemple__Expander__

#include <fstream>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <stack>
#include <string>

#include "ArgList.h"
#include "InStream.h"
#include "Macro.h"
#include "Position.h"

namespace stemple
{
	class Expander
	{
	public:
		Expander ();

		virtual ~Expander ();

		std::string Expand (const std::string &input);

		bool Expand (std::istream &input, const std::string &inputName, std::ostream &output);

		void SetMacro (const std::string &name, const std::string &body, bool simple = false);

		void SetSpecialChars (char escape, char intro, char open, char argSep, char close);

	protected:
		struct Mods
		{
			bool TrimArgs = true;
			bool IgnoreCase = false;
			bool ExpandArgs = true;
			bool Quote = false;
			Mods (bool trimArgs = true) : TrimArgs(trimArgs), IgnoreCase(false), ExpandArgs(true), Quote(false)
			{
			}
		};

		std::string expand (const std::string &input, const std::string &source);

		void expand (std::ostream &output);

		bool processDirective (const Position &introPos);

		std::string collectString (const std::string &delims, bool expand = true);

		ArgList collectArgs (bool trim, bool expand = true);

		std::string trimWhitespace (const std::string &s);

		std::string escapeString (const std::string &s);

		enum Token { ARGS, ASSIGN, APPEND, MOD, SIMPLE_ASSIGN, SIMPLE_APPEND, CLOSE, END, ERR };

		Token collectMods (Mods &mods);

		Token getToken ();

		inline InStream &currentStream ()
		{
			return *inStreams.front();	// TODO: What if inStreams is empty?
		}

		InStream *findStream (std::function<bool(const std::shared_ptr<InStream> &ptr)> pred);
		InStream *findStreamWithNamePrefix (const std::string &prefix);
		InStream *findStreamWithArgs ();
		InStream *findStreamWithPath ();

		const std::path getCurrentPath ();

		bool get (char &c, bool expand = true);

		int peek ();

		bool good ();

		bool eof ();

		bool putback (const char &c);

		bool putback (const std::string &s, const std::string &streamName, const ArgList &args = {});

		bool do_if (const ArgList &args, const Mods &mods);
		bool do_else (const ArgList &args, const Mods &mods);
		bool do_elseif (const ArgList &args, const Mods &mods);
		bool do_endif (const ArgList &args, const Mods &mods);
		bool do_env (const ArgList &args, const Mods &mods);
		bool do_include (const ArgList &args, const Mods &mods);
		bool do_equal (const ArgList &args, const Mods &mods);
		bool do_notequal (const ArgList &args, const Mods &mods);
		bool do_match (const ArgList &args, const Mods &mods);
		bool do_and (const ArgList &args, const Mods &mods);
		bool do_or (const ArgList &args, const Mods &mods);
		bool do_not (const ArgList &args, const Mods &mods);
		bool do_defined (const ArgList &args, const Mods &mods);

		std::list<std::shared_ptr<InStream>> inStreams;
		std::map<std::string, Macro> macros;
		std::map<std::string, std::function<bool(const ArgList &, const Mods &)>> builtins;

		char introChar;				// The start of a directive. Default: '$'
		char openChar;				// The start of a directive body. Default: '('
		char closeChar;				// The end of a directive. Default: ')'
		char argSepChar;			// Separator between arguments. Default: ','
		char escapeChar;			// Escapes other special chars. Default '$'
		char modsChar;				// The start of modifiers to expansion. Default ':'
		std::string nameEndChars;	// Set of terminating chars
		std::string argEndChars;	// Set of terminating chars
		std::string textEndChars;	// Set of terminating chars
		std::string modEndChars;	// Set of terminating chars
		bool trimArgs;				// Trim whitespace from argument strings by default
		int skipping;				// Skipping output and most expansion because we are in a false branch of a block if/elseif/else
		int invoking;				// In the middle of gathering a macro invocation
		bool wasEscaped;			// Last character returned by get() was escaped

		// A stack of descriptors for processing nested block ifs/elseifs/elses
		struct IfContext
		{
			enum Phase { ElseOrEnd, EndOnly};
			Phase phase;
			bool branchTaken;
			bool isSkipping;
		};
		std::stack<IfContext> ifContext;
	};
}

#endif // __stemple__Expander__