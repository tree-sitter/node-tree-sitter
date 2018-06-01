declare module "tree-sitter" {
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
    lengthRemoved: number;
    lengthAdded: number;
    startPosition: Point;
    extentRemoved: Point;
    extentAdded: Point;
  };

  export type Logger = (
    message: string,
    params: {[param: string]: string},
    type: "parse" | "lex"
  ) => void;

  export interface Input {
    seek(index: number): void;
    read(): any;
  };

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
  };

  export interface TreeCursor {
    nodeType: string;
    nodeIsNamed: boolean;
    startPosition: Point;
    endPosition: Point;
    startIndex: number;
    endIndex: number;

    public gotoParent(): boolean;
    public gotoFirstChild(): boolean;
    public gotoNextSibling(): boolean;
  };

  export interface Tree {
    public readonly rootNode: SyntaxNode;

    public edit(delta: Edit): Document;
    public walk(): TreeCursor;
    public getChangedRanges(other: Tree): Range[];
  };

  export class Parser {
    public parse(input: string | Input, previousTree?: Tree): Tree;
    public getLanguage(): any;
    public setLanguage(language: any);
    public getLogger(): Logger;
    public setLogger(logFunc: Logger): void;
  };

  export = Parser;
}
