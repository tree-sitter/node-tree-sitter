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

  export interface Node {
    id: number;
    isNamed: boolean;
    type: string;
    startPosition: Point;
    endPosition: Point;
    children: NodeArray;
    startIndex: number;
    endIndex: number;
    parent: Node | null;
    namedChildren: NodeArray;
    firstChild: Node | null;
    lastChild: Node | null;
    firstNamedChild: Node | null;
    lastNamedChild: Node | null;
    nextSibling: Node | null;
    nextNamedSibling: Node | null;
    previousSibling: Node | null;
    previousNamedSibling: Node | null;

    isValid(): boolean;
    hasError(): boolean;
    hasChanges(): boolean;
    toString(): string;
    descendantForIndex(index: number): Node;
    descendantForIndex(startIndex: number, endIndex: number): Node;
    namedDescendantForIndex(index: number): Node;
    namedDescendantForIndex(startIndex: number, endIndex: number): Node;
    descendantForPosition(position: Point): Node;
    descendantForPosition(startPosition: Point, endPosition: Point): Node;
    namedDescendantForPosition(position: Point): Node;
    namedDescendantForPosition(startPosition: Point, endPosition: Point): Node;
  };

  type ArrayCallback<TItem, TArray, TReturn> = (
    node: TItem,
    index?: number,
    array?: TArray
  ) => TReturn;

  export interface NodeArray extends ArrayLike<Node>, Iterable<Node> {
    map<T>(callback: ArrayCallback<Node, NodeArray, void>, thisArg?: any): T[];
    every(callback: ArrayCallback<Node, NodeArray, void>, thisArg?: any): void;
    filter(callback: ArrayCallback<Node, NodeArray, boolean>, thisArg?: any): Node[];
    find(callback: ArrayCallback<Node, NodeArray, boolean>, thisArg?: any): Node | undefined,
    findIndex(callback: ArrayCallback<Node, NodeArray, boolean>, thisArg?: any): number;
    forEach(callback: ArrayCallback<Node, NodeArray, void>, thisArg?: any): void;
    indexOf(searchElement: Node, fromIndex?: number): number;
    reduce<T>(callback: (accumulator: T, current: Node, currentIndex?: number, array?: NodeArray) => T, initialValue?: T): T;
    reduceRight<T>(callback: (accumulator: T, current: Node, currentIndex?: number, array?: NodeArray) => T, initialValue?: T): T;
    slice(begin?: number, end?: number): Node[],
    some(callback: ArrayCallback<Node, NodeArray, boolean>, thisArg?: any): boolean,
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
    public readonly rootNode: Node;

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
