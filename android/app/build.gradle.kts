plugins {
    id("com.android.application")
}

val ciVersionCode = System.getenv("GITHUB_RUN_NUMBER")?.toIntOrNull()?.coerceAtLeast(2) ?: 2

android {
    namespace = "com.thabombs.pokeemeraldnative"
    compileSdk = 35
    ndkVersion = "27.2.12479018"

    signingConfigs {
        create("testDebug") {
            // Public, development-only key. Keeping it stable lets test APKs
            // update in place across ephemeral GitHub Actions runners.
            storeFile = rootProject.file("pokeemerald-test.keystore")
            storePassword = "android"
            keyAlias = "pokeemerald-test"
            keyPassword = "android"
        }
    }

    defaultConfig {
        applicationId = "com.thabombs.pokeemeraldnative"
        minSdk = 26
        targetSdk = 35
        versionCode = ciVersionCode
        versionName = "0.2.$ciVersionCode"

        externalNativeBuild {
            cmake {
                arguments += listOf("-DANDROID_STL=none")
            }
        }

        ndk {
            abiFilters += listOf("arm64-v8a", "armeabi-v7a", "x86_64")
        }
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.22.1"
        }
    }

    buildTypes {
        debug {
            isJniDebuggable = true
            signingConfig = signingConfigs.getByName("testDebug")
        }
        release {
            isMinifyEnabled = false
        }
    }
}
