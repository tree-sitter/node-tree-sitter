declare module "tree-sitter" {
  export class Document {
    // Getters
    public rootNode: ASTNode | null;

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

  export interface ASTNode {
    // Getters
    id: number;
    isNamed: boolean;
    type: string;
    startPosition: Position;
    endPosition: Position;
    children: ASTNodeArray;
    startIndex: number;
    endIndex: number;
    parent: ASTNode | null;
    namedChildren: ASTNodeArray;
    firstChild: ASTNode | null;
    lastChild: ASTNode | null;
    firstNamedChild: ASTNode | null;
    lastNamedChild: ASTNode | null;
    nextSibling: ASTNode | null;
    nextNamedSibling: ASTNode | null;
    previousSibling: ASTNode | null;
    previousNamedSibling: ASTNode | null;

    // Methods
    isValid(): boolean;
    hasError(): boolean;
    hasChanges(): boolean;
    toString(): string;
    descendantForIndex(index: number): ASTNode;
    descendantForIndex(startIndex: number, endIndex: number): ASTNode;
    namedDescendantForIndex(index: number): ASTNode;
    namedDescendantForIndex(startIndex: number, endIndex: number): ASTNode;
    descendantForPosition(position: Position): ASTNode;
    descendantForPosition(startPosition: Position, endPosition: Position): ASTNode;
    namedDescendantForPosition(position: Position): ASTNode;
    namedDescendantForPosition(startPosition: Position, endPosition: Position): ASTNode;
  }

  type ArrayCallback<TItem, TArray, TReturn> = (node: TItem, index?: number, array?: TArray) => TReturn

  export interface ASTNodeArray extends ArrayLike<ASTNode>, Iterable<ASTNode> {
    map<T>(callback: ArrayCallback<ASTNode, ASTNodeArray, void>, thisArg?: any): T[];
    every(callback: ArrayCallback<ASTNode, ASTNodeArray, void>, thisArg?: any): void;
    filter(callback: ArrayCallback<ASTNode, ASTNodeArray, boolean>, thisArg?: any): ASTNode[];
    find(callback: ArrayCallback<ASTNode, ASTNodeArray, boolean>, thisArg?: any): ASTNode | undefined,
    findIndex(callback: ArrayCallback<ASTNode, ASTNodeArray, boolean>, thisArg?: any): number;
    forEach(callback: ArrayCallback<ASTNode, ASTNodeArray, void>, thisArg?: any): void;
    indexOf(searchElement: ASTNode, fromIndex?: number): number;
    reduce<T>(callback: (accumulator: T, current: ASTNode, currentIndex?: number, array?: ASTNodeArray) => T, initialValue?: T): T;
    reduceRight<T>(callback: (accumulator: T, current: ASTNode, currentIndex?: number, array?: ASTNodeArray) => T, initialValue?: T): T;
    slice(begin?: number, end?: number): ASTNode[],
    some(callback: ArrayCallback<ASTNode, ASTNodeArray, boolean>, thisArg?: any): boolean,
  }
}
