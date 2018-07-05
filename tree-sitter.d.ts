declare module "tree-sitter" {
  class Parser {
    parse(input: string | Parser.Input, previousTree?: Parser.Tree): Parser.Tree;
    getLanguage(): any;
    setLanguage(language: any);
    getLogger(): Parser.Logger;
    setLogger(logFunc: Parser.Logger): void;
  }

  namespace Parser {
    export type Point = {
      row: number;
      column: number;
    };

    export type Range = {
      start: Point;
      end: Point;
    };

    export type Edit = {
      startIndex: number;
      oldEndIndex: number;
      newEndIndex: number;
      startPosition: Point;
      oldEndPosition: Point;
      newEndPosition: Point;
    };

    export type Logger = (
      message: string,
      params: {[param: string]: string},
      type: "parse" | "lex"
    ) => void;

    export interface Input {
      seek(index: number): void;
      read(): any;
    }

    export interface SyntaxNode {
      isNamed: boolean;
      type: string;
      startPosition: Point;
      endPosition: Point;
      children: Array<SyntaxNode>;
      namedChildren: Array<SyntaxNode>;
      startIndex: number;
      endIndex: number;
      parent: SyntaxNode | null;
      firstChild: SyntaxNode | null;
      lastChild: SyntaxNode | null;
      firstNamedChild: SyntaxNode | null;
      lastNamedChild: SyntaxNode | null;
      nextSibling: SyntaxNode | null;
      nextNamedSibling: SyntaxNode | null;
      previousSibling: SyntaxNode | null;
      previousNamedSibling: SyntaxNode | null;

      isValid(): boolean;
      hasError(): boolean;
      hasChanges(): boolean;
      toString(): string;
      descendantForIndex(index: number): SyntaxNode;
      descendantForIndex(startIndex: number, endIndex: number): SyntaxNode;
      namedDescendantForIndex(index: number): SyntaxNode;
      namedDescendantForIndex(startIndex: number, endIndex: number): SyntaxNode;
      descendantForPosition(position: Point): SyntaxNode;
      descendantForPosition(startPosition: Point, endPosition: Point): SyntaxNode;
      namedDescendantForPosition(position: Point): SyntaxNode;
      namedDescendantForPosition(startPosition: Point, endPosition: Point): SyntaxNode;
      descendantsOfType(type: String, startPosition?: Point, endPosition?: Point): Array<SyntaxNode>;
    }

    export interface TreeCursor {
      nodeType: string;
      nodeIsNamed: boolean;
      startPosition: Point;
      endPosition: Point;
      startIndex: number;
      endIndex: number;

      gotoParent(): boolean;
      gotoFirstChild(): boolean;
      gotoFirstChildForIndex(index: number): boolean;
      gotoNextSibling(): boolean;
    }

    export interface Tree {
      readonly rootNode: SyntaxNode;

      edit(delta: Edit): Tree;
      walk(): TreeCursor;
      getChangedRanges(other: Tree): Range[];
      getEditedRange(other: Tree): Range;
    }
  }

  export = Parser
}
