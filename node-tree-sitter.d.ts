declare module "tree-sitter" {
  export class Document {
    // Getters
    public rootNode: AstNode | null;

    // Methods
    public setLanguage(module: any): void;
    public setInputString(input: string): void;
    public parse(): void;
    public getLogger(): LoggerFunc;
    public setLogger(logFunc: LoggerFunc): void;
    public getInput(): Input | null;
    public setInput(input: Input): void;
    public edit(delta: EditDelta): Document;
    public invalidate(): void;
  }

  type LoggerFunc = (message: string, params: {[param: string]: string}, type: "parse" | "lex") => void;

  type EditDelta = {
    startIndex: number;
    lengthRemoved: number;
    lengthAdded: number;
    startPosition: Position;
    extentRemoved: Position;
    extentAdded: Position;
  };

  export type Position = {
    row: number;
    column: number;
  };

  export type Input = {
    seek(index: number): void;
    read(): any;
  }

  export interface AstNode {
    // Getters
    id: number;
    isNamed: boolean;
    type: string;
    startPosition: Position;
    endPosition: Position;
    children: AstNodeArray;
    startIndex: number;
    endIndex: number;
    parent: AstNode | null;
    namedChildren: AstNodeArray;
    firstChild: AstNode | null;
    lastChild: AstNode | null;
    firstNamedChild: AstNode | null;
    lastNamedChild: AstNode | null;
    nextSibling: AstNode | null;
    nextNamedSibling: AstNode | null;
    previousSibling: AstNode | null;
    previousNamedSibling: AstNode | null;

    // Methods
    isValid(): boolean;
    hasError(): boolean;
    hasChanges(): boolean;
    toString(): string;
    descendantForIndex(index: number): AstNode;
    descendantForIndex(startIndex: number, endIndex: number): AstNode;
    namedDescendantForIndex(index: number): AstNode;
    namedDescendantForIndex(startIndex: number, endIndex: number): AstNode;
    descendantForPosition(position: Position): AstNode;
    descendantForPosition(startPosition: Position, endPosition: Position): AstNode;
    namedDescendantForPosition(position: Position): AstNode;
    namedDescendantForPosition(startPosition: Position, endPosition: Position): AstNode;
  }

  type ArrayCallback<TItem, TArray, TReturn> = (node: TItem, index?: number, array?: TArray) => TReturn

  export interface AstNodeArray extends ArrayLike<AstNode> {
    map<T>(callback: ArrayCallback<AstNode, AstNodeArray, void>, thisArg?: any): T[];
    every(callback: ArrayCallback<AstNode, AstNodeArray, void>, thisArg?: any): void;
    filter(callback: ArrayCallback<AstNode, AstNodeArray, boolean>, thisArg?: any): AstNode[];
    find(callback: ArrayCallback<AstNode, AstNodeArray, boolean>, thisArg?: any): AstNode | undefined,
    findIndex(callback: ArrayCallback<AstNode, AstNodeArray, boolean>, thisArg?: any): number;
    forEach(callback: ArrayCallback<AstNode, AstNodeArray, void>, thisArg?: any): void;
    indexOf(searchElement: AstNode, fromIndex?: number): number;
    reduce<T>(callback: (accumulator: T, current: AstNode, currentIndex?: number, array?: AstNodeArray) => T, initialValue?: T): T;
    reduceRight<T>(callback: (accumulator: T, current: AstNode, currentIndex?: number, array?: AstNodeArray) => T, initialValue?: T): T;
    slice(begin?: number, end?: number): AstNode[],
    some(callback: ArrayCallback<AstNode, AstNodeArray, boolean>, thisArg?: any): boolean,
  }
}
