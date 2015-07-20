%% `THG_slides.cls'
%% Copyright 2011 by 
%  Prof. Dr. Dr. Frank Schweitzer,
%  Dr. Antonios Garas,
%  Alexander Grimm,
%  Chair of Systems Design, ETH Zurich
%  Kreuzplatz 5, CH-8032 Zurich
%
% Copyright 2015 by
% Jerome Soumagne 
% Modified original ETHclass to handle lualatex/true type fonts and THG
%
% This class can be redistributed and/or modified under the terms
% of the GNU Public License, version 2.
% For the context of the licence, program shall refer to all source
% files distributed with this stylefile. Note that this does not refer
% to your documents.

% This is a style file for creating LaTeX beamer presentations
% in the THG wide corporate design. 
%
% The following options can be used when including the package:
%
%   totalframes  prints the total number of frames
%   first        uses special first frame
%   firstalt     uses special first frame (alternative version)
%   last         uses special last frame
%   firstsupp    suppresses footer on first frame
%   lastsupp     suppresses footer on last frame
%   navigation   displays navigation symbols
%
%%

%% ========================================================================
%% Admin
%% ========================================================================
\def\fileversion{1.0}
\def\filedate{2015/07/16}
\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{THG_slides}[\filedate\space beamer class by Jerome Soumagne (v\fileversion)]

%% --- Declaration of options ---
\DeclareOption*{\PassOptionsToClass{\CurrentOption}{beamer}}

%% --- Execution of options ---
\ProcessOptions
\LoadClass{beamer}

\RequirePackage{ifpdf}
\RequirePackage{ifthen}
\RequirePackage{layouts}
\RequirePackage{ifluatex}

\ifluatex
  % Font
  \usefonttheme{professionalfonts}
  \RequirePackage{luatextra, xunicode, unicode-math}
  \defaultfontfeatures{Ligatures=TeX}
  \setmainfont[
    UprightFont={*},
    BoldFont={*-Bold},
    ItalicFont={*-Italic},
    BoldItalicFont={*-BoldItalic},
    SlantedFont={*-Italic},
    BoldSlantedFont={*-BoldItalic}
  ]{CMU Serif}
  \setsansfont[
    UprightFont={*},
    BoldFont={*-Bold},
    ItalicFont={*-Oblique},
    BoldItalicFont={*-BoldOblique}
  ]{CMU Sans Serif}
  \setmathfont[math-style=ISO, bold-style=ISO]{Asana Math}
  \setmonofont[
    UprightFont={*-Regular},
    BoldFont={*-Bold},
    ItalicFont={*-Italic},
    BoldItalicFont={*-BoldItalic}
  ]{CMU Typewriter}
\else
  \RequirePackage{times}
  \RequirePackage[scaled=.95]{helvet}
  \RequirePackage{courier}
  \newcommand{\lgblksquare}{\blacksquare}
\fi

%% Graphics
\ifpdf
  \graphicspath{{./pics/}{./plots/}}
  \DeclareGraphicsExtensions{.pdf,.jpeg,.jpg,.png}
  % For transparency
  \pdfpageattr{/Group <</S /Transparency /I true /CS /DeviceRGB>>}
\fi

\setlength{\leftmargini}{1.75pt} % indention of item order 1
\setlength{\leftmarginii}{16pt} % indention of item order 2
\setlength{\leftmarginiii}{16pt} % indention of item order 3
\setlength{\labelsep}{10pt} %% Separation page to item

%% Size of items
\def\Tiny{\fontsize{4.25pt}{4.25pt}\selectfont}

%% Itemizing Symbol: Square
\setbeamertemplate{itemize items}[square]

%% Position and Size of items in all three orders
\useitemizeitemtemplate{%
    \Tiny\raise1.5pt\hbox{\color{itemblue}$\lgblksquare$}%
}
\usesubitemizeitemtemplate{%
    \Tiny\raise0.5pt\hbox{\color{itemblue}$\lgblksquare$}%
}
\usesubsubitemizeitemtemplate{%
    \Tiny\raise0.5pt\hbox{\color{itemblue}$\lgblksquare$}%
}

%% Titlepage
\newcommand{\totalframes}{}
\newcommand{\normalbg}{\includegraphics[width=\paperwidth]{template/normalbg}}
\newcommand{\firstbg}{\normalbg}
\newcommand{\lastbg}{\includegraphics[width=\paperwidth]{template/normalbg}}

%% Position of date / institute / framenumber  
\newcommand{\normalfooter}{%
\leavevmode%
  \hbox{
    \begin{beamercolorbox}[wd=.25\paperwidth,ht=2.5ex,dp=1.125ex,left]{author in head/foot}%
      \qquad\color{white}{\insertdate}%
    \end{beamercolorbox}%

    \begin{beamercolorbox}[wd=.5\paperwidth,ht=2.5ex,dp=1.125ex,center]{title in head/foot}%
      \color{white}{\inserttitle}%
    \end{beamercolorbox}%

    \begin{beamercolorbox}[wd=.2\paperwidth,ht=2.5ex,dp=1.125ex,right]{title in head/foot}%
      \color{white}{\insertframenumber\totalframes}%
    \end{beamercolorbox}%
  }}
\newcommand{\firstfooter}{\normalfooter}
\newcommand{\lastfooter}{\normalfooter}
\newcommand{\navsymb}{\setbeamertemplate{navigation symbols}{}}
\newcommand*\also[1]{\gdef\@also{#1}\global\let\insertalso\@also}%
\newcommand{\usespecialtitlepagetemplate}{
\makeatletter
\setbeamertemplate{title page}%{default}[1][]
{
  \vspace{25pt}
  \begin{beamercolorbox}[wd=0.99\paperwidth,ht=15.8ex,dp=1ex,sep=1.7ex,center,shadow=true]{title}
  %\vskip0.2em
    \usebeamerfont{title}
    {\fontsize{13pt}{4pt}\textbf{\inserttitle}\par}
    \ifx\insertsubtitle\@empty%
    \else%
      \vskip0.5em%
      {\usebeamerfont{subtitle}\usebeamercolor[fg]{subtitle}\insertsubtitle\par}%
    \fi%     
   \end{beamercolorbox}%
  %\vskip0.5em\par
  \vspace{12pt}
   {\usebeamercolor[fg]{titlegraphic}\inserttitlegraphic\par}
  \vskip0.5em\par
   \begin{beamercolorbox}[wd=0.99\paperwidth,ht=1ex,dp=1ex,sep=2.7ex,center,shadow=true]{author}
    {\fontsize{9.5pt}{4pt}\usebeamerfont{author}{\textbf{\insertauthor}}}
   \end{beamercolorbox}
   \begin{beamercolorbox}[wd=0.99\paperwidth,ht=1ex,dp=1ex,sep=2.7ex,center,shadow=true]{institute}
     {\fontsize{9.5pt}{4pt}\usebeamerfont{author}{\insertinstitute}}
   \end{beamercolorbox}
  \vskip0.5em\par
   \begin{beamercolorbox}[wd=0.99\paperwidth,ht=1ex,dp=1ex,sep=2.7ex,center,shadow=true]{date}
     {\fontsize{9.5pt}{4pt}\usebeamerfont{author}{\insertdate}}
   \end{beamercolorbox}
\vskip1.5em
\vfill
}
\makeatother
}

%% Handout option
\DeclareOption{handout}{\gdef\beamer@currentmode{handout}}

%% Information floor
\DeclareOption{totalframes}{\renewcommand{\totalframes}{/\inserttotalframenumber}}
\DeclareOption{last}{\renewcommand{\lastbg}{\includegraphics[width=\paperwidth]{template/normalbg}}}
\DeclareOption{first}{
  \renewcommand{\firstbg}{\includegraphics[width=\paperwidth]{template/firstbg}}
  \usespecialtitlepagetemplate
}
\DeclareOption{firstalt}{
  \renewcommand{\firstbg}{}
  \usespecialtitlepagetemplate
}

\DeclareOption{firstsupp}{\renewcommand{\firstfooter}{}}
\DeclareOption{lastsupp}{\renewcommand{\lastfooter}{}}

% execute options
\ProcessOptions

\setbeamertemplate{footline}
{%
\ifthenelse{\equal{\insertframenumber}{1}}{%
    \firstfooter%
  }{%
    \ifthenelse{\equal{\insertframenumber}{\inserttotalframenumber}}{%
      \lastfooter%
    }{%
      \normalfooter%
    }
  }
}

%%% Frametitle position each slide
\setbeamertemplate{frametitle}
{
  \vspace{4pt}
  \begin{beamercolorbox}[wd=.99\paperwidth,sep=8pt,right]{frametitle}%
    {\fontsize{13.9pt}{8pt}\textbf{\insertframetitle}\par}
  \end{beamercolorbox}%
  \vspace{-4pt}
}

\usebackgroundtemplate{
  \ifthenelse{\equal{\insertframenumber}{1}}{%
    \firstbg%
  }{%
    \ifthenelse{\equal{\insertframenumber}{\inserttotalframenumber}}{%
      \lastbg%
    }{%
      \normalbg%
    }
  }
}

%%% Notes
%\mode<notes>

\mode<presentation>
{
  \useoutertheme{default}    % empty
%  \useoutertheme{infolines} % simple but bland
%  \useoutertheme{split}     % ok if compress option used
%  \useoutertheme{shadow}    % way too much space used -- ok with option 'compress'
  %\useoutertheme{shadow}   
  %\setbeamercovered{transparent} % or whatever (possibly just delete it)
  %\useoutertheme[subsection=false]{miniframes}
}

% Color definitions
\usecolortheme[RGB={0,43,94}]{structure}
\definecolor{blue}{RGB}{0,43,94}
\definecolor{black}{RGB}{0,0,0}
\definecolor{white}{RGB}{255,255,255}
%\definecolor{red}{RGB}{161,80,0}
\definecolor{itemblue}{RGB}{0,80,145} %% Color of item squares (different
                                %% blue)

\setbeamercolor{block title}{use=blue,fg=blue.fg,bg=blue.fg!20!bg}
\setbeamercolor{block title alerted}{use=alerted text,fg=alerted text.fg,bg=alerted text.fg!20!bg}
\setbeamercolor{block title example}{use=example text,fg=example text.fg,bg=example text.fg!20!bg}

\setbeamercolor{block body}{parent=normal text,use=block title,bg=block title.bg!50!bg}
\setbeamercolor{block body alerted}{parent=normal text,use=block title alerted}
\setbeamercolor{block body example}{parent=normal text,use=block title example}

\setbeamercolor{normal text}{fg=black,bg=white}
\setbeamercolor{alerted text}{fg=red}

%%
%% End of file
