%% thesis
%%   LaTeX class for writing RFCs
%% Author: Jerome Soumagne <jsoumagne@hdfgroup.org>
%%

%% ========================================================================
%% Admin
%% ========================================================================
\def\fileversion{1.0}
\def\filedate{2014/07/22}
\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{THG_RFC}[\filedate\space RFC class by Jerome Soumagne (v\fileversion)]

%% ========================================================================
%% Option parsing
%% ========================================================================
%% Conditional packages (can be disabled)
\def\@empty{}
\def\@yes{yes}
\def\@requireamsmath{yes}
\def\@requirebooktabs{yes}
\def\@requirehyper{}

%% Use AMS math packages
\DeclareOption{noams}{%
  \def\@requireamsmath{}}
\DeclareOption{ams}{%
  \def\@requireamsmath{yes}}

%% Change footnote markers to alphabetic
\DeclareOption{alphafoot}{%
  \DeclareRobustCommand{\thefootnote}{\alph{footnote}}%
}

% Use hyperref package (with decent options)
\DeclareOption{hyper}{
  \def\@requirehyper{yes}}

% Use hyperref package (with decent options) only if output is PDF
\DeclareOption{hyperpdf}{
  \ifx\pdfoutput\undefined\else%
  \def\@requirehyper{yes}%
  \fi%
}

%% Don't break enumerations (etc.) across pages in
%% an ugly manner
\clubpenalty = 10000
\widowpenalty = 10000

%% Use nice-looking tables
\DeclareOption{booktabs}{%
  \def\@requirebooktabs{yes}}
\DeclareOption{nobooktabs}{%
  \def\@requirebooktabs{}}

\DeclareOption{letterpaper}{%
  \PassOptionsToClass{letterpaper}{scrartcl}%
}
\DeclareOption{legalpaper}{%
  \PassOptionsToClass{legalpaper}{scrartcl}%
}
\DeclareOption{executivepaper}{%
  \PassOptionsToClass{executivepaper}{scrartcl}%
}

%% Process the options (no package requirement allowed above here)
\ProcessOptions

%% ========================================================================
%% Base class and packages
%% ========================================================================

%% Base class
\LoadClass[11pt]{scrartcl}

% Use English by default
\RequirePackage[english]{babel}

%% Graphics
\ifx\pdfoutput
  \RequirePackage[dvips]{graphicx}
\else
  \RequirePackage[pdftex]{graphicx}
  \DeclareGraphicsExtensions{.pdf,.jpeg,.jpg,.png}
  % For transparency
  \pdfpageattr{/Group <</S /Transparency /I true /CS /DeviceRGB>>}
\fi

%% If building with PDFLaTeX, use microtype spacing adjustments
\RequirePackage{microtype}

%% Need to be able to locally change the text width.
\RequirePackage{changepage}

%% Used to center the variable width page quote (must be version >= 0.9a)
\RequirePackage{varwidth}[2003/03/10]

%% AMS math package (optional)
\ifx\@requireamsmath\@empty\else%
  \RequirePackage{amsmath}
  \AtEndOfPackage{%
    %% Displayed maths environments re-defined to be the same as AMS' align
    \renewenvironment{displaymath}{\align}{\endalign\ignorespacesafterend}
    \newenvironment{displaymath*}%
      {\expandafter\let\expandafter\@oldoldalignstar \csname align*\endcsname \@oldoldalignstar}%
      {\expandafter\let\expandafter\@oldoldendalignstar \csname endalign*\endcsname \@oldoldendalignstar \ignorespacesafterend}
    \renewenvironment{equation}{\align}{\endalign\ignorespacesafterend}
    \renewenvironment{equation*}%
      {\expandafter\let\expandafter\@oldoldalignstar \csname align*\endcsname \@oldoldalignstar}%
      {\expandafter\let\expandafter\@oldoldendalignstar \csname endalign*\endcsname \@oldoldendalignstar \ignorespacesafterend}
  }%
\fi

%% Use the \title and \author arguments to build the front page etc.
%% Re-define \title and \author to store their args in \thetitle, \theauthor variables
\renewcommand*\title[1]{\gdef\@title{#1}\global\let\thetitle\@title}%
\def\theauthors{}
\renewcommand*\author[1]{\gdef\@author{#1}\global\let\theauthor\@author
\g@addto@macro\theauthors{#1 \\}
}%
\renewcommand*\date[1]{\gdef\@date{#1}\global\let\thedate\@date}%
\newcommand*\rfcversion[1]{\gdef\@rfcversion{#1}\global\let\therfcversion\@rfcversion}%

\def\revisions{}
\newcounter{revision}
% Adds a new reviewer
\def\revision#1#2{\g@addto@macro\revisions{%
 {\itshape{#1}:} & {#2} \\}
}
%\newcommand*\rfcrevision[1]{\gdef\@rfcrevision{#1}\global\let\therfcrevision\@rfcrevision}%

%% Nice tables (optional)
\ifx\@requirebooktabs\@empty\else%
  \RequirePackage{booktabs}
  \AtEndOfClass{%
    %% Tabular environment, defined to use the booktabs facilities
    \let\@oldtabular\tabular
    \let\@oldendtabular\endtabular
    \renewenvironment{tabular}[1]{%
      \@oldtabular{@{}#1@{}}%
      %\toprule%
    }{%
      %\bottomrule%
      \@oldendtabular%
      \ignorespacesafterend%
    }
    %%
  }%
\fi

%% The right way to do custom spacing
\RequirePackage{setspace}
%% \singlespacing, \onehalfspacing, \doublespacing
%% \begin{spacing}{2.5} ...

%% Funky headers
\RequirePackage{fancyhdr}

% For colors
\RequirePackage{xcolor}
\definecolor{rulecolor}{RGB}{73,126,191}

% LastPage
\RequirePackage{lastpage}

%% Verbatim things
\RequirePackage{verbatim}

%% TODO
\RequirePackage{todonotes}

% Float package
\RequirePackage{float}

%% Re-defines captions of figures and tables
%% TODO: make configurable / optional?
\RequirePackage[format=hang,labelfont=bf,labelsep=space,font=small]{caption}

% Subfigures
\RequirePackage[labelfont=bf,labelsep=space,list=true]{subcaption}

%% Hyperlinks (optional, has to come early in the package list)
\ifx\@requirehyper\@empty\else%
\RequirePackage[colorlinks,
  citecolor=black,%
  filecolor=black,%
  linkcolor=black,%
  urlcolor=black,%
  bookmarks]{hyperref}
\fi

%% ========================================================================
%% Length definitions
%% ========================================================================

\setlength{\parindent}{22pt}          % tweak the initial indent in paras
\setlength{\parskip}{2pt}             % tweak the intra-para gap size
\setlength{\topmargin}{0in}
\setlength{\textheight}{592pt}
\addtolength{\textwidth}{\oddsidemargin}
\addtolength{\textwidth}{\evensidemargin}
\setlength{\marginparwidth}{57pt}
%\setlength{\footskip}{30pt}

%% Increase inner margins
\AtEndOfClass{%
  \setlength{\oddsidemargin}{0pt}
  \setlength{\evensidemargin}{0pt}
}

%% Redefine bits of document shape
\renewcommand{\topfraction}{0.95}%
\renewcommand{\bottomfraction}{0.95}%
\renewcommand{\textfraction}{0.05}%
\renewcommand{\arraystretch}{1.25}%

%% Reset rule widths and the header width
\renewcommand{\headwidth}{\textwidth}%
\renewcommand{\headrulewidth}{0.5pt}%
\renewcommand{\footrulewidth}{0.5pt}%
\addtolength{\headheight}{2.5pt}%
%\addtolength{\headsep}{20pt}%

%% Figure widths
\newlength{\smallfigwidth}
\newlength{\mediumfigwidth}
\newlength{\largefigwidth}
\newlength{\hugefigwidth}
\setlength{\smallfigwidth}{0.45\textwidth}
\setlength{\mediumfigwidth}{0.6\textwidth}
\setlength{\largefigwidth}{0.75\textwidth}
\setlength{\hugefigwidth}{0.9\textwidth}

%% Figure width aliases
\newlength{\littlefigwidth}
\newlength{\bigfigwidth}
\setlength{\littlefigwidth}{\smallfigwidth}
\setlength{\bigfigwidth}{\largefigwidth}

%% Logo height
\newlength{\logoheight}
\setlength{\logoheight}{2.5\baselineskip}

%% ========================================================================
%% Headers and footers
%% ========================================================================
%% No header or footer on the title page
\AtBeginDocument{\thispagestyle{first}}
%% Use funky headers and footers
\pagestyle{fancy}

%% Normal headers and footers (headers are all like RH pages for oneside)
\fancyhf{}%
\newcommand{\headerfont}{\small}
\fancyhead[R]{\headerfont RFC THG \therfcversion}%
\fancyhead[L]{\headerfont\thedate}%
\fancyfoot[L]{\includegraphics[height=\logoheight]{THG_LOGO}}
\fancyfoot[R]{\vspace*{-\logoheight}{\headerfont Page~\thepage~of~\pageref{LastPage}}}

\let\@oldheadrule\headrule
\renewcommand{\headrule}{{\color{rulecolor}\@oldheadrule}}
\let\@oldfootrule\footrule
\renewcommand{\footrule}{{\color{rulecolor}\@oldfootrule}}

%% Headers and footers for plain (blank) pages
\fancypagestyle{first}{%
  \fancyhf{}%
  \fancyhead[R]{\headerfont RFC THG \therfcversion}%
  \fancyhead[L]{\headerfont \thedate}%
  \renewcommand{\headrulewidth}{0pt}%
  \fancyfoot[L]{\includegraphics[height=\logoheight]{THG_LOGO}}
  \fancyfoot[R]{\vspace*{-\logoheight}{\headerfont Page~\thepage~of~\pageref{LastPage}}}
}

%% ========================================================================
%% Internal environments and commands
%% ========================================================================
\AtEndOfClass{%
  \usepackage[babel]{csquotes}[2007/03/25]
  %% Provide dumb quoting commands in case the csquotes package hasn't been used.
  \providecommand{\enquote}[1]{``#1''}%
  \providecommand{\foreignquote}[1]{``#1''}%
}

%% ========================================================================
%% Public environments and commands
%% ========================================================================

%% Makes a title
\DeclareRobustCommand{\titlepage}[2][]{%
  \begingroup%
  \sffamily%
  \begin{singlespacing}
  \begin{center}%
    \vspace*{10pt}{\LARGE\bfseries{RFC: \thetitle}}\\%
    \vspace*{20pt}{\bfseries{\theauthors}}%
    {\color{rulecolor}\rule{\headwidth}{\headrulewidth}}%
  \end{center}%
  \end{singlespacing}
  \endgroup%
}
\renewcommand{\maketitle}[1]{\titlepage{}}

%% Environment for abstract
\renewenvironment{abstract}{%
  \vspace*{-7pt}%
}{%
  \ignorespacesafterend%
  \\%
  {\color{rulecolor}\rule{\headwidth}{\headrulewidth}}%
}

%% A quote on its own page
\DeclareRobustCommand{\makerevisions}[1]{%
  \begin{flushleft}%
    \begin{tabular}{ll}%
      \revisions%
    \end{tabular}%
  \end{flushleft}%
}


%% Bold inline section heading with a finishing colon
\DeclareRobustCommand{\verysubsection}[1]{{\noindent\textbf{#1:}}}

%% Table environment
\let\@oldtable\table
\let\@oldendtable\endtable
\renewenvironment{table}[1][tbp]{%
  \vspace{0.4cm}%
  \@oldtable[#1]%
  \begin{center}%
}{%
  \end{center}%
  \@oldendtable%
  \ignorespacesafterend%
}

%% Figure environment
\let\@oldfigure\figure
\let\@oldendfigure\endfigure
\renewenvironment{figure}[1][htbp]{
  \@oldfigure[#1]%
  \begin{center}%
}{%
  \end{center}%
  \@oldendfigure%
  \ignorespacesafterend%
}

%% Figure* environment (lots of \expandafter and \csname gubbins reqd... *sigh*)
\expandafter\let\expandafter\@oldfigurestar \csname figure*\endcsname
\expandafter\let\expandafter\@oldendfigurestar \csname endfigure*\endcsname
\renewenvironment{figure*}[1][htbp]{
  \@oldfigurestar[#1]%
  \begin{center}%
}{%
  \end{center}%
  \@oldendfigurestar%
  \ignorespacesafterend%
}

%% In-document references
\DeclareRobustCommand{\Chapter}{Chapter}
\DeclareRobustCommand{\Section}{Section}
\DeclareRobustCommand{\Appendix}{Appendix}
\DeclareRobustCommand{\Figure}{Figure}
\DeclareRobustCommand{\Table}{Table}
\DeclareRobustCommand{\Equation}{equation}
\DeclareRobustCommand{\Reference}{reference}
%% Standard way to refer to a page number
\DeclareRobustCommand{\Page}{page}

%% Reference terms with built-in reference
\DeclareRobustCommand{\ChapterRef}[1]{\Chapter~\ref{#1}}
\DeclareRobustCommand{\SectionRef}[1]{\Section~\ref{#1}}
\DeclareRobustCommand{\AppendixRef}[1]{\Appendix~\ref{#1}}
\DeclareRobustCommand{\FigureRef}[1]{\Figure~\ref{#1}}
\DeclareRobustCommand{\TableRef}[1]{\Table~\ref{#1}}
\DeclareRobustCommand{\EquationRef}[1]{\Equation~\eqref{#1}}
\DeclareRobustCommand{\ReferenceRef}[1]{\Reference~\cite{#1}}
\DeclareRobustCommand{\PageRef}[1]{\Page~\pageref{#1}}

