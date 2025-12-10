sequenceDiagram
    participant Pass as OptPass
    participant I as infer()
    participant C as Canonicalizer
    participant P as Parser
    participant E as Enumerator
    participant Redis as Redis Cache

    Note over Pass,Redis: Cache Lookup Phase
    Pass->>I: optimize(OriginalFunc, Instr)
    I->>I: Clone Module
    I->>C: canonicalize(ClonedFunc)
    Note right of C: Stores permutation<br/>as metadata on ClonedFunc
    C-->>I: ClonedFunc (canonical)
    I->>I: Generate bytecode key<br/>from canonical form
    I->>Redis: hGet(canonical_key)
    
    alt Cache Hit
        Redis-->>I: rewrite_string (canonical form)
        I->>P: parse(ClonedFunc, rewrite_string)
        Note right of P: Parser uses canonical<br/>function context
        P-->>I: Rewrite (canonical)
        I->>C: adaptRewrite(Rewrite, ClonedFunc→OriginalFunc)
        Note right of C: Use inverse permutation<br/>to map arguments back
        C-->>I: Rewrite (adapted to original)
        I-->>Pass: Rewrite (ready to apply)
    else Cache Miss
        Redis-->>I: null
        Note over I,E: Synthesis Phase
        I->>E: solve(ClonedFunc, Instr)
        Note right of E: Synthesizer works<br/>on canonical function
        E-->>I: Rewrite (canonical)
        I->>Redis: hSet(canonical_key, rewrite_string)
        Redis-->>I: success
        I->>C: adaptRewrite(Rewrite, ClonedFunc→OriginalFunc)
        C-->>I: Rewrite (adapted to original)
        I-->>Pass: Rewrite (ready to apply)
    end
    
    Note over Pass: Apply rewrite to OriginalFunc
    Pass->>Pass: Apply transformation