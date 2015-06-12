        #ifdef __18C658
              include <P18C658.INC>
        #endif

        #ifdef __18C858
              include <P18C858.INC>
        #endif

        #ifdef __18F248
              include <P18F248.INC>
        #ifndef RX0DBEN
                RX0DBEN         EQU  H'0002'    ;For backward compatibility
        #endif
        #endif

        #ifdef __18F258
              include <P18F258.INC>
        #ifndef RX0DBEN
                RX0DBEN         EQU  H'0002'    ;For backward compatibility
        #endif
        #endif

        #ifdef __18F448
              include <P18F448.INC>
        #ifndef RX0DBEN
                RX0DBEN         EQU  H'0002'    ;For backward compatibility
        #endif

        #endif

        #ifdef __18F458
              include <P18F458.INC>
        #ifndef RX0DBEN
                RX0DBEN         EQU  H'0002'    ;For backward compatibility
        #endif
        #endif

