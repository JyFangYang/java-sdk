package org.fisco.bcos.sdk.codec.datatypes.generated;

import java.math.BigInteger;
import org.fisco.bcos.sdk.codec.datatypes.Int;

/**
 * Auto generated code.
 *
 * <p><strong>Do not modifiy!</strong>
 *
 * <p>Please use AbiTypesGenerator in the <a
 * href="https://github.com/web3j/web3j/tree/master/codegen">codegen module</a> to update.
 */
public class Int24 extends Int {
    public static final Int24 DEFAULT = new Int24(BigInteger.ZERO);

    public Int24(BigInteger value) {
        super(24, value);
    }

    public Int24(long value) {
        this(BigInteger.valueOf(value));
    }
}
