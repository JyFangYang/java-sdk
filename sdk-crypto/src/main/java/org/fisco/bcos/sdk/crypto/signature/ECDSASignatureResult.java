/**
 * Copyright 2014-2020 [fisco-dev]
 *
 * <p>Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of the License at
 *
 * <p>http://www.apache.org/licenses/LICENSE-2.0
 *
 * <p>Unless required by applicable law or agreed to in writing, software distributed under the
 * License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 */
package org.fisco.bcos.sdk.crypto.signature;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import org.fisco.bcos.sdk.crypto.exceptions.SignatureException;
import org.fisco.bcos.sdk.utils.Hex;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class ECDSASignatureResult extends SignatureResult {
    protected static Logger logger = LoggerFactory.getLogger(SignatureResult.class);
    protected byte v;
    protected static int VBASE = 27;

    public ECDSASignatureResult(byte v, byte[] r, byte[] s) {
        super(r, s);
        this.v = v;
    }

    public ECDSASignatureResult(final String signatureResult) {
        super(signatureResult);
        if (this.signatureBytes.length != 65) {
            throw new SignatureException(
                    "Invalid signature for invalid length " + this.signatureBytes.length);
        }
        this.v = this.signatureBytes[64];
    }

    /**
     * covert signatureResult into String
     *
     * @return the signature string with [r, s, v]
     */
    @Override
    public String convertToString() {
        byte[] SignatureBytes = new byte[65];
        System.arraycopy(this.r, 0, SignatureBytes, 0, 32);
        System.arraycopy(this.s, 0, SignatureBytes, 32, 32);
        SignatureBytes[64] = this.v;
        return Hex.toHexString(SignatureBytes);
    }

    @Override
    public String toString() {
        return convertToString();
    }

    @Override
    public byte[] encode() {
        ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
        try {
            byteArrayOutputStream.write(this.r);
            byteArrayOutputStream.write(this.s);
            byteArrayOutputStream.write(this.v);
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
        return byteArrayOutputStream.toByteArray();
    }

    public byte getV() {
        return this.v;
    }

    public void setV(byte v) {
        this.v = v;
    }
}
